//////////////////////////////////////
// ChocoLang Amour Lib for GUI
// GTK4 Bindings for ChocoLang
//////////////////////////////////////

#include "choco_gui.h"
#include <iostream>

struct Value {
    enum Type { NUMBER, STRING, BOOL, ARRAY, STRUCT, LAMBDA, NIL } type;
    double num;
    std::string str;
    bool boolean;
    std::vector<Value> array;
    std::unordered_map<std::string, Value> structFields;
    std::string structType;
    
    std::vector<std::string> lambdaParams;
    size_t lambdaBodyStart;
    size_t lambdaBodyEnd;
    std::unordered_map<std::string, Value> closureCaptures;

    Value() : type(NIL), num(0), boolean(false), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(double n) : type(NUMBER), num(n), boolean(false), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(const std::string& s) : type(STRING), str(s), num(0), boolean(false), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(bool b) : type(BOOL), num(0), boolean(b), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(const std::vector<Value>& arr) : type(ARRAY), num(0), boolean(false), array(arr), lambdaBodyStart(0), lambdaBodyEnd(0) {}

    std::string toString() const {
        switch (type) {
            case NUMBER: {
                if (num == static_cast<int>(num)) {
                    return std::to_string(static_cast<int>(num));
                }
                std::string s = std::to_string(num);
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') s.pop_back();
                return s;
            }
            case STRING: return str;
            case BOOL: return boolean ? "true" : "false";
            case ARRAY: {
                std::string result = "[";
                for (size_t i = 0; i < array.size(); i++) {
                    result += array[i].toString();
                    if (i < array.size() - 1) result += ", ";
                }
                result += "]";
                return result;
            }
            case STRUCT: {
                std::string result = structType + " { ";
                bool first = true;
                for (const auto& field : structFields) {
                    if (!first) result += ", ";
                    result += field.first + ": " + field.second.toString();
                    first = false;
                }
                result += " }";
                return result;
            }
            case LAMBDA: return "<lambda>";
            case NIL: return "nil";
        }
        return "";
    }
    
    std::string getType() const {
        switch (type) {
            case NUMBER: return "number";
            case STRING: return "string";
            case BOOL: return "bool";
            case ARRAY: return "array";
            case STRUCT: return structType.empty() ? "struct" : structType;
            case LAMBDA: return "lambda";
            case NIL: return "nil";
        }
        return "unknown";
    }
};

class RuntimeError : public std::runtime_error {
public:
    int line;
    RuntimeError(const std::string& msg, int line_num) 
        : std::runtime_error(msg), line(line_num) {}
};

class Interpreter;

ChocoGUI* ChocoGUI::instance = nullptr;

ChocoGUI* ChocoGUI::getInstance(int argc, char** argv) {
    if (!instance) {
        instance = new ChocoGUI(argc, argv);
    }
    return instance;
}

void ChocoGUI::on_activate(GtkApplication* app, gpointer user_data) {
}

void ChocoGUI::on_button_clicked(GtkButton* button, gpointer user_data) {
    std::string* widgetId = static_cast<std::string*>(user_data);
    ChocoGUI* gui = ChocoGUI::getInstance();
    gui->executeCallback(*widgetId, "clicked");
}

void ChocoGUI::on_entry_changed(GtkEditable* editable, gpointer user_data) {
    std::string* widgetId = static_cast<std::string*>(user_data);
    ChocoGUI* gui = ChocoGUI::getInstance();
    gui->executeCallback(*widgetId, "changed");
}

void ChocoGUI::on_window_close(GtkWindow* window, gpointer user_data) {
    std::string* widgetId = static_cast<std::string*>(user_data);
    ChocoGUI* gui = ChocoGUI::getInstance();
    gui->executeCallback(*widgetId, "close");
}

void ChocoGUI::executeCallback(const std::string& widgetId, const std::string& event) {
    auto it = widgets.find(widgetId);
    if (it != widgets.end()) {
        auto callbackIt = it->second.callbacks.find(event);
        if (callbackIt != it->second.callbacks.end() && interpreter && callbackFunc) {
            std::string funcName = callbackIt->second;
            std::cout << "Callback triggered: " << funcName << std::endl;
            
            try {
                std::vector<Value> args;
                Value result = callbackFunc(interpreter, funcName, args, 0);
            } catch (const std::exception& e) {
                std::cerr << "Error executing callback: " << e.what() << std::endl;
            }
        } else {
            if (!interpreter) {
                std::cerr << "Error: No interpreter set for callback" << std::endl;
            }
            if (!callbackFunc) {
                std::cerr << "Error: No callback function set" << std::endl;
            }
        }
    }
}

Value ChocoGUI::gui_init(const std::vector<Value>& args, int line) {
    std::string appId = "com.chocolang.app";
    if (args.size() > 0 && args[0].type == Value::STRING) {
        appId = args[0].str;
    }
    
    // Create application with DEFAULT_FLAGS
    app = gtk_application_new(appId.c_str(), G_APPLICATION_DEFAULT_FLAGS);
    
    if (!app) {
        throw RuntimeError("Failed to create GTK application", line);
    }
    
    // Register the application but don't activate yet
    GError* error = nullptr;
    if (!g_application_register(G_APPLICATION(app), nullptr, &error)) {
        std::string errMsg = error ? error->message : "Unknown error";
        if (error) g_error_free(error);
        throw RuntimeError("Failed to register application: " + errMsg, line);
    }
    
    return Value(true);
}

Value ChocoGUI::gui_window(const std::vector<Value>& args, int line) {
    if (!app) {
        throw RuntimeError("GUI not initialized. Call gui_init() first", line);
    }
    
    std::string title = "ChocoLang Window";
    int width = 400;
    int height = 300;
    std::string id = "main_window";
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        title = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::NUMBER) {
        width = static_cast<int>(args[1].num);
    }
    if (args.size() > 2 && args[2].type == Value::NUMBER) {
        height = static_cast<int>(args[2].num);
    }
    if (args.size() > 3 && args[3].type == Value::STRING) {
        id = args[3].str;
    }
    
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    
    mainWindow = window;
    
    WidgetData data;
    data.widget = window;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_button(const std::vector<Value>& args, int line) {
    if (args.size() < 1 || args[0].type != Value::STRING) {
        throw RuntimeError("gui_button() requires label as first argument", line);
    }
    
    std::string label = args[0].str;
    std::string id = "button_" + std::to_string(widgets.size());
    
    if (args.size() > 1 && args[1].type == Value::STRING) {
        id = args[1].str;
    }
    
    GtkWidget* button = gtk_button_new_with_label(label.c_str());
    
    WidgetData data;
    data.widget = button;
    data.id = id;
    widgets[id] = data;
    
    std::string* widgetId = new std::string(id);
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), widgetId);
    
    return Value(id);
}

Value ChocoGUI::gui_label(const std::vector<Value>& args, int line) {
    std::string text = "";
    std::string id = "label_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        text = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::STRING) {
        id = args[1].str;
    }
    
    GtkWidget* label = gtk_label_new(text.c_str());
    
    WidgetData data;
    data.widget = label;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_entry(const std::vector<Value>& args, int line) {
    std::string placeholder = "";
    std::string id = "entry_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        placeholder = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::STRING) {
        id = args[1].str;
    }
    
    GtkWidget* entry = gtk_entry_new();
    if (!placeholder.empty()) {
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), placeholder.c_str());
    }
    
    WidgetData data;
    data.widget = entry;
    data.id = id;
    widgets[id] = data;
    
    std::string* widgetId = new std::string(id);
    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), widgetId);
    
    return Value(id);
}

Value ChocoGUI::gui_box(const std::vector<Value>& args, int line) {
    std::string orientation = "vertical";
    int spacing = 5;
    std::string id = "box_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        orientation = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::NUMBER) {
        spacing = static_cast<int>(args[1].num);
    }
    if (args.size() > 2 && args[2].type == Value::STRING) {
        id = args[2].str;
    }
    
    GtkOrientation orient = (orientation == "horizontal" || orientation == "h") 
                            ? GTK_ORIENTATION_HORIZONTAL 
                            : GTK_ORIENTATION_VERTICAL;
    
    GtkWidget* box = gtk_box_new(orient, spacing);
    
    WidgetData data;
    data.widget = box;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_add(const std::vector<Value>& args, int line) {
    if (args.size() < 2 || args[0].type != Value::STRING || args[1].type != Value::STRING) {
        throw RuntimeError("gui_add() requires two widget IDs (parent, child)", line);
    }
    
    std::string parentId = args[0].str;
    std::string childId = args[1].str;
    
    auto parentIt = widgets.find(parentId);
    auto childIt = widgets.find(childId);
    
    if (parentIt == widgets.end()) {
        throw RuntimeError("Parent widget '" + parentId + "' not found", line);
    }
    if (childIt == widgets.end()) {
        throw RuntimeError("Child widget '" + childId + "' not found", line);
    }
    
    GtkWidget* parent = parentIt->second.widget;
    GtkWidget* child = childIt->second.widget;
    
    if (GTK_IS_WINDOW(parent)) {
        gtk_window_set_child(GTK_WINDOW(parent), child);
    }
    else if (GTK_IS_BOX(parent)) {
        gtk_box_append(GTK_BOX(parent), child);
    }
    else if (GTK_IS_FRAME(parent)) {
        gtk_frame_set_child(GTK_FRAME(parent), child);
    }
    else {
        throw RuntimeError("Cannot add child to this widget type", line);
    }
    
    return Value(true);
}

Value ChocoGUI::gui_set_text(const std::vector<Value>& args, int line) {
    if (args.size() < 2 || args[0].type != Value::STRING || args[1].type != Value::STRING) {
        throw RuntimeError("gui_set_text() requires widget ID and text", line);
    }
    
    std::string widgetId = args[0].str;
    std::string text = args[1].str;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    GtkWidget* widget = it->second.widget;
    
    if (GTK_IS_LABEL(widget)) {
        gtk_label_set_text(GTK_LABEL(widget), text.c_str());
    }
    else if (GTK_IS_BUTTON(widget)) {
        gtk_button_set_label(GTK_BUTTON(widget), text.c_str());
    }
    else if (GTK_IS_ENTRY(widget)) {
        GtkEntryBuffer* buffer = gtk_entry_get_buffer(GTK_ENTRY(widget));
        gtk_entry_buffer_set_text(buffer, text.c_str(), -1);
    }
    else if (GTK_IS_WINDOW(widget)) {
        gtk_window_set_title(GTK_WINDOW(widget), text.c_str());
    }
    else if (GTK_IS_TEXT_VIEW(widget)) {
        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        gtk_text_buffer_set_text(buffer, text.c_str(), -1);
    }
    else {
        throw RuntimeError("Cannot set text on this widget type", line);
    }
    
    return Value(true);
}

Value ChocoGUI::gui_get_text(const std::vector<Value>& args, int line) {
    if (args.size() < 1 || args[0].type != Value::STRING) {
        throw RuntimeError("gui_get_text() requires widget ID", line);
    }
    
    std::string widgetId = args[0].str;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    GtkWidget* widget = it->second.widget;
    
    if (GTK_IS_LABEL(widget)) {
        return Value(std::string(gtk_label_get_text(GTK_LABEL(widget))));
    }
    else if (GTK_IS_BUTTON(widget)) {
        return Value(std::string(gtk_button_get_label(GTK_BUTTON(widget))));
    }
    else if (GTK_IS_ENTRY(widget)) {
        GtkEntryBuffer* buffer = gtk_entry_get_buffer(GTK_ENTRY(widget));
        return Value(std::string(gtk_entry_buffer_get_text(buffer)));
    }
    else if (GTK_IS_TEXT_VIEW(widget)) {
        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        gchar* text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        std::string result(text);
        g_free(text);
        return Value(result);
    }
    else {
        throw RuntimeError("Cannot get text from this widget type", line);
    }
}

Value ChocoGUI::gui_on(const std::vector<Value>& args, int line) {
    if (args.size() < 3 || args[0].type != Value::STRING || 
        args[1].type != Value::STRING || args[2].type != Value::STRING) {
        throw RuntimeError("gui_on() requires widget ID, event name, and callback function name", line);
    }
    
    std::string widgetId = args[0].str;
    std::string event = args[1].str;
    std::string callback = args[2].str;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    it->second.callbacks[event] = callback;
    
    return Value(true);
}

Value ChocoGUI::gui_show(const std::vector<Value>& args, int line) {
    if (args.size() < 1 || args[0].type != Value::STRING) {
        throw RuntimeError("gui_show() requires widget ID", line);
    }
    
    std::string widgetId = args[0].str;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    gtk_widget_set_visible(it->second.widget, TRUE);
    
    return Value(true);
}

Value ChocoGUI::gui_run(const std::vector<Value>& args, int line) {
    if (!app) {
        throw RuntimeError("GUI not initialized. Call gui_init() first", line);
    }
    
    if (!mainWindow) {
        throw RuntimeError("No window created. Call gui_window() first", line);
    }
    
    // Show all widgets
    gtk_widget_set_visible(mainWindow, TRUE);
    
    // Present the window
    gtk_window_present(GTK_WINDOW(mainWindow));
    
    // Hold the application (keeps it running)
    g_application_hold(G_APPLICATION(app));
    
    // Run the main loop WITHOUT calling g_application_run
    // because we've already registered the application
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    
    // Release and cleanup
    g_application_release(G_APPLICATION(app));
    g_object_unref(app);
    
    return Value(0.0);
}

Value ChocoGUI::gui_quit(const std::vector<Value>& args, int line) {
    if (mainWindow) {
        gtk_window_destroy(GTK_WINDOW(mainWindow));
    }
    return Value(true);
}

Value ChocoGUI::gui_checkbox(const std::vector<Value>& args, int line) {
    std::string label = "";
    std::string id = "checkbox_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        label = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::STRING) {
        id = args[1].str;
    }
    
    GtkWidget* checkbox = gtk_check_button_new_with_label(label.c_str());
    
    WidgetData data;
    data.widget = checkbox;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_textview(const std::vector<Value>& args, int line) {
    std::string id = "textview_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        id = args[0].str;
    }
    
    GtkWidget* textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    
    WidgetData data;
    data.widget = textview;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_frame(const std::vector<Value>& args, int line) {
    std::string label = "";
    std::string id = "frame_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        label = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::STRING) {
        id = args[1].str;
    }
    
    GtkWidget* frame = gtk_frame_new(label.c_str());
    
    WidgetData data;
    data.widget = frame;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_separator(const std::vector<Value>& args, int line) {
    std::string orientation = "horizontal";
    std::string id = "separator_" + std::to_string(widgets.size());
    
    if (args.size() > 0 && args[0].type == Value::STRING) {
        orientation = args[0].str;
    }
    if (args.size() > 1 && args[1].type == Value::STRING) {
        id = args[1].str;
    }
    
    GtkOrientation orient = (orientation == "horizontal" || orientation == "h") 
                            ? GTK_ORIENTATION_HORIZONTAL 
                            : GTK_ORIENTATION_VERTICAL;
    
    GtkWidget* separator = gtk_separator_new(orient);
    
    WidgetData data;
    data.widget = separator;
    data.id = id;
    widgets[id] = data;
    
    return Value(id);
}

Value ChocoGUI::gui_set_sensitive(const std::vector<Value>& args, int line) {
    if (args.size() < 2 || args[0].type != Value::STRING || args[1].type != Value::BOOL) {
        throw RuntimeError("gui_set_sensitive() requires widget ID and boolean", line);
    }
    
    std::string widgetId = args[0].str;
    bool sensitive = args[1].boolean;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    gtk_widget_set_sensitive(it->second.widget, sensitive ? TRUE : FALSE);
    
    return Value(true);
}

Value ChocoGUI::gui_get_checked(const std::vector<Value>& args, int line) {
    if (args.size() < 1 || args[0].type != Value::STRING) {
        throw RuntimeError("gui_get_checked() requires widget ID", line);
    }
    
    std::string widgetId = args[0].str;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    if (!GTK_IS_CHECK_BUTTON(it->second.widget)) {
        throw RuntimeError("Widget is not a checkbox", line);
    }
    
    bool checked = gtk_check_button_get_active(GTK_CHECK_BUTTON(it->second.widget));
    return Value(checked);
}

Value ChocoGUI::gui_set_checked(const std::vector<Value>& args, int line) {
    if (args.size() < 2 || args[0].type != Value::STRING || args[1].type != Value::BOOL) {
        throw RuntimeError("gui_set_checked() requires widget ID and boolean", line);
    }
    
    std::string widgetId = args[0].str;
    bool checked = args[1].boolean;
    
    auto it = widgets.find(widgetId);
    if (it == widgets.end()) {
        throw RuntimeError("Widget '" + widgetId + "' not found", line);
    }
    
    if (!GTK_IS_CHECK_BUTTON(it->second.widget)) {
        throw RuntimeError("Widget is not a checkbox", line);
    }
    
    gtk_check_button_set_active(GTK_CHECK_BUTTON(it->second.widget), checked ? TRUE : FALSE);
    
    return Value(true);
}
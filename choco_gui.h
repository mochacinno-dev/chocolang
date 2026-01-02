//////////////////////////////////////
// ChocoLang Amour Lib for GUI
// GTK4 Bindings for ChocoLang
//////////////////////////////////////

#ifndef CHOCO_GUI_H
#define CHOCO_GUI_H

#include <gtk/gtk.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

// Add this typedef before the ChocoGUI class
class Interpreter;
struct Value;

typedef Value (*CallbackFunction)(Interpreter*, const std::string&, const std::vector<Value>&, int);

class ChocoGUI {
private:
    static ChocoGUI* instance;
    Interpreter* interpreter;
    
    typedef Value (*CallbackFunction)(Interpreter*, const std::string&, const std::vector<Value>&, int);
    CallbackFunction callbackFunc;

    struct WidgetData {
        GtkWidget* widget;
        std::string id;
        std::unordered_map<std::string, std::string> callbacks;
    };
    
    std::unordered_map<std::string, WidgetData> widgets;
    GtkApplication* app;
    GtkWidget* mainWindow;
    int argc;
    char** argv;
    
    ChocoGUI(int argc, char** argv) : interpreter(nullptr), callbackFunc(nullptr), app(nullptr), 
                                       mainWindow(nullptr), argc(argc), argv(argv) {}
    
    static void on_activate(GtkApplication* app, gpointer user_data);
    static void on_button_clicked(GtkButton* button, gpointer user_data);
    static void on_entry_changed(GtkEditable* editable, gpointer user_data);
    static void on_window_close(GtkWindow* window, gpointer user_data);
    
    void executeCallback(const std::string& widgetId, const std::string& event);

public:
    static ChocoGUI* getInstance(int argc = 0, char** argv = nullptr);
    void setInterpreter(Interpreter* interp) { interpreter = interp; }
    void setCallbackFunction(CallbackFunction func) { callbackFunc = func; }
    
    Value gui_init(const std::vector<Value>& args, int line);
    Value gui_window(const std::vector<Value>& args, int line);
    Value gui_button(const std::vector<Value>& args, int line);
    Value gui_label(const std::vector<Value>& args, int line);
    Value gui_entry(const std::vector<Value>& args, int line);
    Value gui_box(const std::vector<Value>& args, int line);
    Value gui_add(const std::vector<Value>& args, int line);
    Value gui_set_text(const std::vector<Value>& args, int line);
    Value gui_get_text(const std::vector<Value>& args, int line);
    Value gui_on(const std::vector<Value>& args, int line);
    Value gui_show(const std::vector<Value>& args, int line);
    Value gui_run(const std::vector<Value>& args, int line);
    Value gui_quit(const std::vector<Value>& args, int line);
    
    Value gui_checkbox(const std::vector<Value>& args, int line);
    Value gui_textview(const std::vector<Value>& args, int line);
    Value gui_frame(const std::vector<Value>& args, int line);
    Value gui_separator(const std::vector<Value>& args, int line);
    
    Value gui_set_sensitive(const std::vector<Value>& args, int line);
    Value gui_get_checked(const std::vector<Value>& args, int line);
    Value gui_set_checked(const std::vector<Value>& args, int line);
};

#endif
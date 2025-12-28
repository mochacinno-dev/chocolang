<div align="center">

<img src="assets/banner-logo.png" alt="Tourmaline Logo" width="400"/>

**A Sweet Programming Language**

[![Version](https://img.shields.io/badge/version-2.0.0-chocolate.svg)](https://github.com/yourusername/chocolang)
[![License](https://img.shields.io/badge/license-MIT-brown.svg)](LICENSE)
[![Made with](https://img.shields.io/badge/made%20by-Coffeeshop%20Development-saddlebrown.svg)](https://github.com/yourusername/chocolang)

**Simple and Expressive** • **Fast to Learn** • **Great Control**
</div>

---

## 🚀 Quick Start

### Installation

```bash
git clone https://github.com/mochacinno-dev/ChocoLang.git
cd ChocoLang
g++ -o choco main.cpp -std=c++11
```

### Your First Program

Create a file `hello.choco`:

```choco
// Hello World in Choco
let name = "World";
puts "Hello, #{name}!";

fn greet(person) {
    return "Welcome to ChocoLang, #{person}!";
}

puts greet(name);
```

Run it:

```bash
./choco hello.choco
```


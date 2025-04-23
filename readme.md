````markdown
# testProject

A minimal C/C++ scaffold using CMake, supporting both native and WebAssembly (WASM) builds with WebGL2 rendering. Ideal for developers looking to prototype graphics apps in C/C++ and run them either as a native executable or in the browser via Emscripten.

## Acknowledgements

- [Awesome Readme Templates](https://awesomeopensource.com/project/elangosundar/awesome-README-templates)
- [Awesome README](https://github.com/matiassingers/awesome-readme)
- [How to write a Good readme](https://bulldogjob.com/news/449-how-to-write-a-good-readme-for-your-github-project)

## API Reference

#### `int initWebGL(void)`

Initializes a WebGL2 context on the `<canvas id="canvas">` element.  
**Returns:** `1` on success, `0` on failure.

#### `void startMainLoop(void)`

Starts the render loop; each frame simply clears the screen to a teal color.

#### `void myFunction(void)`

An EMSCRIPTEN_KEEPALIVE function exposed in the WASM build. Can be invoked from JavaScript via `Module.ccall('myFunction', null, [], []);`.

## Appendix

- You can customize the HTML shell in `html_template/index.html` or provide your own via `html_template/`.
- To adjust canvas dimensions, edit the `<canvas>` attributes in your HTML shell.

## Authors

- [@yourusername](https://github.com/yourusername)

## Badges

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

## Color Reference

| Color      | Hex                                                                |
| ---------- | ------------------------------------------------------------------ |
| Teal Clear | ![#0a9396](https://via.placeholder.com/10/0a9396?text=+) `#0a9396` |

## Contributing

Contributions are always welcome!  
Please see `CONTRIBUTING.md` for guidelines and `CODE_OF_CONDUCT.md` for behavior expectations.

## Demo

After building the WASM target, run:

```bash
./build_and_run.sh
```
````

Then open your browser at `http://localhost:8000`.

## Deployment

This project is intended for local development; there’s no external deployment pipeline. Use the `build_and_run.sh` script to serve locally.

## Documentation

All usage instructions are in this README. For deeper dives, inspect the CMakeLists and source headers in `src/`.

## Environment Variables

_None required._

## FAQ

#### How do I change the canvas size?

Edit the `width` and `height` attributes on the `<canvas>` in your HTML template (`html_template/index.html` or custom shell).

#### Which browsers work?

Any modern browser with WebAssembly and WebGL2 support (e.g., Chrome, Firefox, Edge).

## Features

- **Native build** via CMake (Ninja or Make)
- **WASM build** via Emscripten with WebGL2/ES3 support
- Auto-generated or custom HTML shell
- Simple render loop clearing to a teal background
- `build_and_run.sh` helper script with:
  - `--no-serve` (build only)
  - `--watch` (live-reload on changes)
  - Customizable build directory, build type, project name

## Feedback

If you have any feedback, please open an issue or contact me at `me@example.com`.

## 🚀 About Me

I'm a Software Engineer passionate about cross-platform C/C++ and WebAssembly.

## 🔗 Links

[![GitHub](https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/yourusername)

## 🛠 Skills

C, C++, CMake, Emscripten, WebGL2, Bash

## Installation

```bash
git clone https://github.com/yourusername/testProject.git
cd testProject
chmod +x build_and_run.sh
```

## Lessons Learned

- Streamlined cross-compilation with CMake + Emscripten
- Automated live-reload development workflow
- Simple WebGL2 setup in C/C++ via the Emscripten HTML5 API

## License

[MIT](https://choosealicense.com/licenses/mit/)

![Logo](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/th5xamgrr6se0x5ro4g6.png)

## Optimizations

- Auto-detects build system (Ninja vs. Make)
- Port-conflict handling and browser auto-launch

## Related

- [Awesome README](https://github.com/matiassingers/awesome-readme)

## Roadmap

- Add modularized Emscripten build mode
- Support more advanced rendering demos

## Run Locally

```bash
./build_and_run.sh
```

## Screenshots

![App Screenshot](https://via.placeholder.com/468x300?text=App+Screenshot+Here)

## Support

Open an issue or email `support@example.com`.

## Tech Stack

**Native:** C, C++  
**Web:** WebAssembly (Emscripten), WebGL2

## Running Tests

_No tests available._

## Usage/Examples

```bash
# Native build, run executable
./build_and_run.sh --no-serve

# WASM build with live-reload
./build_and_run.sh --watch
```

## Used By

This project is used by individual developers and as a learning template.

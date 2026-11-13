(function () {
  if (
    typeof customElements !== "undefined" &&
    !customElements.get("f3d-viewer")
  ) {
    let canvasCounter = 0;

    // Redirect logs to browser console
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.QUIET, false);
    Module.Log.forward((level, message) => {
      if (level === Module.LogVerboseLevel.ERROR) console.error(message);
      else if (level === Module.LogVerboseLevel.WARN) console.warn(message);
      else if (level === Module.LogVerboseLevel.INFO) console.info(message);
    });

    // load all plugins
    Module.Engine.autoloadPlugins();

    // Define the custom F3D viewer element
    class F3DViewerElement extends HTMLElement {
      constructor() {
        super();
        this.attachShadow({ mode: "open" });
        this.shadowRoot.innerHTML = `
          <style>
            :host {
              display: block;
              width: 100%;
              height: 100%;
              position: relative;
              background: transparent;
            }

            ::slotted(canvas) {
              display: block;
              width: 100%;
              height: 100%;
              outline: none;
            }
          </style>
          <slot></slot>
        `;

        // Create the canvas
        this.canvas = document.createElement("canvas");
        this.canvas.id = `f3d-canvas-${canvasCounter++}`;
        this.enginePromise = null;
        this.engine = null;
        this.module = Module;
        this.resizeObserver = null;
      }

      // function automatically called when a f3d-viewer element is added to the DOM
      connectedCallback() {
        if (!this.canvas) {
          return;
        }

        // removed default context menu on the canvas
        this.canvas.oncontextmenu = (event) => {
          event.preventDefault();
          event.stopPropagation();
        };

        // focus the canvas when the user clicks it so keyboard events are forwarded
        this.canvas.tabIndex = 0;
        this.canvas.addEventListener("mousedown", () => this.canvas.focus());

        if (!this.canvas.isConnected) {
          this.appendChild(this.canvas);
        }

        this.resizeObserver = new ResizeObserver(() => this._resize());
        this.resizeObserver.observe(this);

        this._ensureEngine();
      }

      // function automatically called when a f3d-viewer element is removed from the DOM
      disconnectedCallback() {
        this.resizeObserver?.disconnect();
      }

      // private method to ensure the engine is created and initialized
      // this function triggers the "ready" event once the engine is initialized
      // but before the interactor is started in order to let the user configure interactor bindings/callbacks
      async _ensureEngine() {
        if (!this.enginePromise) {
          this.enginePromise = Promise.resolve().then(() => {
            this.engine = Module.Engine.create(`#${this.canvas.id}`);

            const options = this.engine.getOptions();

            // background must be set to black for proper blending with transparent canvas
            options.setAsString("render.background.color", "#000000");

            // enable DPI awareness
            options.toggle("ui.dpi_aware");

            this._resize();

            this.dispatchEvent(
              new CustomEvent("ready", {
                detail: this.engine,
                bubbles: true,
              }),
            );

            this.engine.getInteractor().start();

            return this.engine;
          });
        }
        return this.enginePromise;
      }

      // helper to handle WebAssembly exceptions and extract their messages
      exceptionMessage(error) {
        let message = "";
        if (error instanceof WebAssembly.RuntimeError) {
          message = this.module.getExceptionMessage(error);
          this.module.decrementExceptionRefcount(error);
        } else {
          message = error.message;
        }
        return message;
      }

      // convenient method to load a model into the viewer
      async load(data) {
        await this._ensureEngine();

        const scene = this.engine.getScene();
        scene.clear();
        scene.addBuffer(data);
        this.engine.getWindow().getCamera().resetToBounds(0.9);
        this.engine.getWindow().render();
      }

      // private method to handle resizing of the viewer and its associated window
      _resize() {
        if (!this.engine || !this.canvas) {
          return;
        }

        const window = this.engine.getWindow();

        // setup the window size based on the canvas size
        const scale = window.getDPIScale();
        window.setSize(
          scale * this.canvas.clientWidth,
          scale * this.canvas.clientHeight,
        );
        window.render();
      }

      static get observedAttributes() {
        return ["src", "up"];
      }

      // called when an observed attribute changes on the f3d-viewer element
      attributeChangedCallback(name, oldValue, newValue) {
        if (oldValue === newValue) {
          return;
        }

        if (name === "src" && newValue) {
          fetch(newValue)
            .then((response) => {
              return response.arrayBuffer();
            })
            .then((arrayBuffer) => {
              this.load(new Uint8Array(arrayBuffer)).catch((error) => {
                console.error("F3D error while loading model:", error);
              });
            });
        }

        if (name === "up" && newValue) {
          this._ensureEngine().then(() => {
            this.engine
              .getOptions()
              .setAsString("scene.up_direction", newValue);
            this.engine.getWindow().render();
          });
        }
      }

      set options(obj) {
        if (this.engine) {
          const options = this.engine.getOptions();
          for (const [key, value] of Object.entries(obj)) {
            options.setAsString(key, value.toString());
          }
        }
      }
    }

    customElements.define("f3d-viewer", F3DViewerElement);
  }
})();

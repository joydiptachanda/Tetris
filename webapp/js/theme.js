(function (app) {
  "use strict";

  class ThemeController {
    constructor(storage) {
      this.storage = storage;
      this.button = document.querySelector("#theme-toggle");
      this.themeColor = document.querySelector('meta[name="theme-color"]');
      this.apply(this.storage.loadTheme());
      this.button.addEventListener("click", () => this.toggle());
    }

    apply(theme) {
      const isDark = theme === "dark";
      document.documentElement.dataset.theme = isDark ? "dark" : "light";
      const label = `Switch to ${isDark ? "light" : "dark"} mode`;
      this.button.setAttribute("aria-label", label);
      this.button.setAttribute("aria-pressed", String(isDark));
      this.button.title = label;
      this.button.textContent = isDark ? "☼" : "☾";
      this.themeColor.content = isDark ? "#151b17" : "#e9eee8";
    }

    toggle() {
      const theme = document.documentElement.dataset.theme === "dark" ? "light" : "dark";
      this.apply(theme);
      this.storage.saveTheme(theme);
    }
  }

  app.ThemeController = ThemeController;
})(window.TetrisApp);
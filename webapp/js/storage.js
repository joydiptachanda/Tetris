(function (app) {
  "use strict";

  const config = app.config;

  function loadRecord() {
    try {
      const saved = JSON.parse(localStorage.getItem(config.recordKey));
      if (saved && Number.isFinite(saved.score) && saved.score >= 0) {
        return { name: typeof saved.name === "string" ? saved.name : "---", score: saved.score };
      }
    } catch (_) {
      // Storage may be unavailable when the file is opened directly.
    }
    return { name: "---", score: 0 };
  }

  function saveRecord(name, score) {
    const record = { name: name.trim() || "---", score };
    try {
      localStorage.setItem(config.recordKey, JSON.stringify(record));
    } catch (_) {
      // Keep the current-session record visible if storage is blocked.
    }
    return record;
  }

  function loadTheme() {
    try {
      const savedTheme = localStorage.getItem(config.themeKey);
      if (savedTheme === "light" || savedTheme === "dark") return savedTheme;
    } catch (_) {
      // Fall back to the system preference when storage is unavailable.
    }
    return window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }

  function saveTheme(theme) {
    try {
      localStorage.setItem(config.themeKey, theme);
    } catch (_) {
      // The theme still changes for this visit if storage is blocked.
    }
  }

  app.storage = {
    loadRecord,
    saveRecord,
    clearRecord: () => saveRecord("---", 0),
    loadTheme,
    saveTheme,
  };
})(window.TetrisApp);
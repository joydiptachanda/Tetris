(function (app) {
  "use strict";

  const game = new app.Game();
  const renderer = new app.Renderer(game);
  const userInterface = new app.Interface(game, renderer, app.storage);
  new app.ThemeController(app.storage);
  userInterface.refresh();
  window.addEventListener("resize", () => userInterface.refresh(), { passive: true });

  window.setInterval(() => {
    if (!userInterface.isModalOpen() && game.step()) userInterface.refresh();
  }, app.config.tickMs);
})(window.TetrisApp);
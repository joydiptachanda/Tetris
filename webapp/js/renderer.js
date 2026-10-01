(function (app) {
  "use strict";

  const config = app.config;

  class Renderer {
    constructor(game) {
      this.game = game;
      this.boardCanvas = document.querySelector("#game-board");
      this.boardContext = this.boardCanvas.getContext("2d");
      this.holdCanvas = document.querySelector("#hold-preview");
      this.nextCanvas = document.querySelector("#next-preview");
      this.holdContext = this.holdCanvas.getContext("2d");
      this.nextContext = this.nextCanvas.getContext("2d");
      this.previewShapes = { hold: undefined, next: undefined };
      this.renderedState = null;
      this.hud = {
        score: document.querySelector("#score-value"),
        level: document.querySelector("#level-value"),
        lines: document.querySelector("#lines-value"),
        lineCount: document.querySelector("#line-count"),
        recordScore: document.querySelector("#record-score"),
        sideRecord: document.querySelector("#side-record"),
        recordName: document.querySelector("#record-name"),
        holdStatus: document.querySelector("#hold-status"),
        status: document.querySelector("#game-status"),
        overlay: document.querySelector("#board-overlay"),
        overlayTitle: document.querySelector("#overlay-title"),
        overlayButton: document.querySelector("#overlay-button"),
        pauseButton: document.querySelector('[data-action="pause"]'),
      };
    }

    drawCell(context, x, y, size, color, ghost = false) {
      const inset = ghost ? 3 : 1.5;
      const width = size - inset * 2;
      if (ghost) {
        context.save();
        context.globalAlpha = 0.62;
        context.strokeStyle = color;
        context.lineWidth = 1.5;
        context.strokeRect(x + inset, y + inset, width, width);
        context.restore();
        return;
      }
      context.fillStyle = color;
      context.fillRect(x + inset, y + inset, width, width);
      context.fillStyle = "rgba(255,255,255,.22)";
      context.fillRect(x + inset, y + inset, width, 3);
      context.fillStyle = "rgba(0,0,0,.16)";
      context.fillRect(x + inset, y + size - inset - 2, width, 2);
    }

    prepareCanvas(canvas, context, width, height) {
      const ratio = window.devicePixelRatio || 1;
      const bounds = canvas.getBoundingClientRect();
      const scaleX = bounds.width / width;
      const scaleY = bounds.height / height;
      const pixelWidth = Math.round(bounds.width * ratio);
      const pixelHeight = Math.round(bounds.height * ratio);
      const resized = canvas.width !== pixelWidth || canvas.height !== pixelHeight;
      if (resized) {
        canvas.width = pixelWidth;
        canvas.height = pixelHeight;
      }
      context.setTransform(ratio * scaleX, 0, 0, ratio * scaleY, 0, 0);
      return resized;
    }

    drawBoard() {
      const game = this.game;
      const width = config.width * config.cellSize;
      const height = config.visibleRows * config.cellSize;
      const context = this.boardContext;
      this.prepareCanvas(this.boardCanvas, context, width, height);
      context.clearRect(0, 0, width, height);
      context.fillStyle = "#111612";
      context.fillRect(0, 0, width, height);
      context.strokeStyle = "#202a22";
      context.lineWidth = 1;
      for (let column = 1; column < config.width; column += 1) {
        context.beginPath();
        context.moveTo(column * config.cellSize + 0.5, 0);
        context.lineTo(column * config.cellSize + 0.5, height);
        context.stroke();
      }
      for (let row = 1; row < config.visibleRows; row += 1) {
        context.beginPath();
        context.moveTo(0, row * config.cellSize + 0.5);
        context.lineTo(width, row * config.cellSize + 0.5);
        context.stroke();
      }

      const hiddenRows = game.clearAnimation && Math.floor(game.clearAnimation.frame / 3) % 2 === 1
        ? game.clearAnimation.rows : null;
      for (let row = 2; row < config.height; row += 1) {
        if (hiddenRows && hiddenRows.includes(row)) continue;
        for (let column = 0; column < config.width; column += 1) {
          const cell = game.field[row][column];
          if (cell) this.drawCell(context, column * config.cellSize, (row - 2) * config.cellSize, config.cellSize, config.colors[cell - 1]);
        }
      }

      if (game.current && game.state === "playing") {
        let ghostY = game.current.y;
        while (game.canPlaceAt(game.current.shape, game.current.rotation, game.current.x, ghostY + 1)) ghostY += 1;
        this.drawPiece(game.current, ghostY, true);
        this.drawPiece(game.current, game.current.y, false);
      }
    }

    drawPiece(piece, originY, ghost) {
      const matrix = config.shapes[piece.shape][piece.rotation];
      const color = config.colors[piece.shape];
      for (let row = 0; row < 4; row += 1) {
        for (let column = 0; column < 4; column += 1) {
          const boardY = originY + row;
          if (matrix[row][column] === "#" && boardY >= 2) {
            this.drawCell(this.boardContext, (piece.x + column) * config.cellSize, (boardY - 2) * config.cellSize, config.cellSize, color, ghost);
          }
        }
      }
    }

    drawPreview(canvas, context, shape, previewName) {
      const width = 120;
      const height = 96;
      const resized = this.prepareCanvas(canvas, context, width, height);
      if (!resized && this.previewShapes[previewName] === shape) return;
      this.previewShapes[previewName] = shape;
      context.clearRect(0, 0, width, height);
      if (shape === null || shape === undefined) return;

      const matrix = config.shapes[shape][0];
      const size = 20;
      const occupied = [];
      for (let row = 0; row < 4; row += 1) {
        for (let column = 0; column < 4; column += 1) {
          if (matrix[row][column] === "#") occupied.push([column, row]);
        }
      }
      const columns = occupied.map(([column]) => column);
      const rows = occupied.map(([, row]) => row);
      const minColumn = Math.min(...columns);
      const maxColumn = Math.max(...columns);
      const minRow = Math.min(...rows);
      const maxRow = Math.max(...rows);
      const offsetX = (width - (maxColumn - minColumn + 1) * size) / 2 - minColumn * size;
      const offsetY = (height - (maxRow - minRow + 1) * size) / 2 - minRow * size;
      for (const [column, row] of occupied) {
        this.drawCell(context, offsetX + column * size, offsetY + row * size, size, config.colors[shape]);
      }
    }

    updateHud(record) {
      const game = this.game;
      this.setText(this.hud.score, this.formatScore(game.score));
      this.setText(this.hud.level, String(game.level).padStart(2, "0"));
      this.setText(this.hud.lines, String(game.lines).padStart(2, "0"));
      this.setText(this.hud.lineCount, `${String(game.lines).padStart(2, "0")} LINES`);
      this.setText(this.hud.recordScore, this.formatScore(record.score));
      this.setText(this.hud.sideRecord, this.formatScore(record.score));
      this.setText(this.hud.recordName, record.name);
      this.setText(this.hud.holdStatus, game.heldShape === null ? "READY TO HOLD" : game.holdUsed ? "USED THIS TURN" : "ON DECK");
      this.drawPreview(this.holdCanvas, this.holdContext, game.heldShape, "hold");
      this.drawPreview(this.nextCanvas, this.nextContext, game.nextShape, "next");
    }

    setText(element, text) {
      if (element.textContent !== text) element.textContent = text;
    }

    formatScore(score) {
      return String(score).padStart(6, "0");
    }

    renderState() {
      const game = this.game;
      if (this.renderedState === game.state) return;
      this.renderedState = game.state;
      const { status, overlay, overlayTitle, overlayButton, pauseButton } = this.hud;
      status.classList.toggle("status-paused", game.state === "paused");
      const labels = { playing: "IN PLAY", paused: "PAUSED", gameover: "GAME OVER", ended: "SESSION ENDED" };
      status.innerHTML = `<i></i> ${labels[game.state]}`;
      overlay.hidden = game.state === "playing";
      overlayTitle.textContent = game.state === "paused" ? "PAUSED" : game.state === "gameover" ? "GAME OVER" : "RUN ENDED";
      overlayButton.dataset.action = game.state === "paused" ? "resume" : "new-game";
      overlayButton.innerHTML = game.state === "paused" ? 'RESUME <span aria-hidden="true">→</span>' : 'PLAY AGAIN <span aria-hidden="true">→</span>';
      pauseButton.innerHTML = `<span>${game.state === "paused" ? "▶" : "Ⅱ"}</span> ${game.state === "paused" ? "RESUME" : "PAUSE"}`;
    }
  }

  app.Renderer = Renderer;
})(window.TetrisApp);
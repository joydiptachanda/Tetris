(function (app) {
  "use strict";

  class Interface {
    constructor(game, renderer, storage) {
      this.game = game;
      this.renderer = renderer;
      this.storage = storage;
      this.record = storage.loadRecord();
      this.dialogOpen = false;
      this.recordPrompted = false;
      this.pendingConfirmation = null;
      this.pointerAction = null;
      this.pointerId = null;
      this.pointerRepeatTimer = null;
      this.suppressedPointerClick = null;
      this.suppressedClickTimer = null;
      this.confirmDialog = document.querySelector("#confirm-dialog");
      this.recordDialog = document.querySelector("#record-dialog");
      this.recordForm = document.querySelector("#record-form");
      this.recordInput = document.querySelector("#record-input");
      this.bindEvents();
    }

    bindEvents() {
      document.addEventListener("keydown", (event) => this.handleKeyDown(event));
      document.addEventListener("click", (event) => this.handleClick(event));
      document.addEventListener("pointerdown", (event) => this.handlePointerDown(event));
      document.addEventListener("pointerup", (event) => this.stopPointerRepeat(event));
      document.addEventListener("pointercancel", (event) => this.stopPointerRepeat(event));
      window.addEventListener("blur", () => this.stopPointerRepeat());
      document.querySelector("#confirm-accept").addEventListener("click", () => this.closeConfirmation(true));
      this.confirmDialog.addEventListener("cancel", (event) => {
        event.preventDefault();
        this.closeConfirmation(false);
      });
      this.recordForm.addEventListener("submit", (event) => {
        event.preventDefault();
        this.submitRecord();
      });
      this.recordDialog.addEventListener("cancel", () => {
        this.record = this.storage.saveRecord("---", this.game.score);
        this.dialogOpen = false;
        this.refresh();
      });
    }

    handleKeyDown(event) {
      if (event.target instanceof HTMLInputElement || this.dialogOpen) return;
      const actionByKey = {
        ArrowLeft: "left", ArrowRight: "right", ArrowDown: "soft-drop",
        z: "rotate-left", Z: "rotate-left", x: "rotate-right", X: "rotate-right",
        " ": "hard-drop", c: "hold", C: "hold", p: "pause", P: "pause",
        r: "restart", R: "restart", h: "clear-record", H: "clear-record", q: "quit", Q: "quit",
      };
      const action = actionByKey[event.key];
      if (!action) return;
      event.preventDefault();
      this.performAction(action);
    }

    handleClick(event) {
      if (!(event.target instanceof Element)) return;
      const actionButton = event.target.closest("[data-action]");
      if (actionButton) {
        if (event.detail > 0 && this.suppressedPointerClick === actionButton.dataset.action) {
          this.suppressedPointerClick = null;
          window.clearTimeout(this.suppressedClickTimer);
          return;
        }
        this.performAction(actionButton.dataset.action);
        return;
      }
      if (event.target.closest("[data-dialog-cancel]")) this.closeConfirmation(false);
      if (event.target.closest("[data-skip-record]")) this.submitRecord();
    }

    handlePointerDown(event) {
      if (!(event.target instanceof Element) || event.button !== 0) return;
      const button = event.target.closest(".move-controls [data-action]");
      if (!button || !["left", "right", "soft-drop"].includes(button.dataset.action)) return;
      event.preventDefault();
      this.stopPointerRepeat();
      window.clearTimeout(this.suppressedClickTimer);
      this.suppressedPointerClick = null;
      const action = button.dataset.action;
      this.pointerAction = action;
      this.pointerId = event.pointerId;
      this.performAction(action);
      this.pointerRepeatTimer = window.setTimeout(() => {
        if (this.pointerAction !== action) return;
        this.performAction(action);
        this.pointerRepeatTimer = window.setInterval(() => {
          if (this.pointerAction === action) this.performAction(action);
        }, 85);
      }, 260);
    }

    stopPointerRepeat(event) {
      if (!this.pointerAction || (event && event.pointerId !== this.pointerId)) return;
      const action = this.pointerAction;
      window.clearTimeout(this.pointerRepeatTimer);
      window.clearInterval(this.pointerRepeatTimer);
      window.clearTimeout(this.suppressedClickTimer);
      this.pointerAction = null;
      this.pointerId = null;
      this.pointerRepeatTimer = null;
      this.suppressedPointerClick = action;
      this.suppressedClickTimer = window.setTimeout(() => {
        this.suppressedPointerClick = null;
      }, 500);
    }

    performAction(action) {
      if (this.dialogOpen) return;
      if (action === "resume") {
        if (this.game.state === "paused") this.game.state = "playing";
      } else if (action === "pause") {
        if (this.game.state === "playing") this.game.state = "paused";
        else if (this.game.state === "paused") this.game.state = "playing";
      } else if (action === "new-game") {
        this.startNewRun();
        return;
      } else if (action === "restart") {
        if (this.game.state === "gameover" || this.game.state === "ended") this.startNewRun();
        else this.requestConfirmation("START A NEW RUN?", "Your current score and board will be cleared.", () => this.startNewRun());
      } else if (action === "clear-record") {
        this.requestConfirmation("CLEAR PERSONAL BEST?", "The saved name and score will be removed.", () => {
          this.record = this.storage.clearRecord();
        });
      } else if (action === "quit") {
        if (this.game.state === "gameover" || this.game.state === "ended") this.game.state = "ended";
        else this.requestConfirmation("END THIS RUN?", "Your current run will be left behind.", () => {
          this.game.state = "ended";
        });
      } else if (action === "left") {
        this.game.movePiece(-1, 0);
      } else if (action === "right") {
        this.game.movePiece(1, 0);
      } else if (action === "soft-drop") {
        this.game.movePiece(0, 1, true);
      } else if (action === "hard-drop") {
        this.game.hardDrop();
      } else if (action === "rotate-left") {
        this.game.rotatePiece(-1);
      } else if (action === "rotate-right") {
        this.game.rotatePiece(1);
      } else if (action === "hold") {
        this.game.holdPiece();
      }
      this.refresh();
    }

    startNewRun() {
      this.game.newRun();
      this.recordPrompted = false;
      this.refresh();
    }

    requestConfirmation(title, message, action) {
      document.querySelector("#confirm-title").textContent = title;
      document.querySelector("#confirm-message").textContent = message;
      this.pendingConfirmation = action;
      this.dialogOpen = true;
      this.confirmDialog.showModal();
    }

    closeConfirmation(accept) {
      const action = this.pendingConfirmation;
      this.pendingConfirmation = null;
      this.dialogOpen = false;
      this.confirmDialog.close();
      if (accept && action) action();
      this.refresh();
    }

    openRecordDialog() {
      document.querySelector("#record-points-value").textContent = this.renderer.formatScore(this.game.score);
      this.recordInput.value = "";
      this.dialogOpen = true;
      this.recordDialog.showModal();
      this.recordInput.focus();
    }

    submitRecord() {
      this.record = this.storage.saveRecord(this.recordInput.value, this.game.score);
      this.dialogOpen = false;
      this.recordDialog.close();
      this.refresh();
    }

    refresh() {
      this.renderer.drawBoard();
      this.renderer.updateHud(this.record);
      this.renderer.renderState();
      if (this.game.state === "gameover" && this.game.score > this.record.score && !this.recordPrompted) {
        this.recordPrompted = true;
        this.openRecordDialog();
      }
    }

    isModalOpen() {
      return this.dialogOpen;
    }
  }

  app.Interface = Interface;
})(window.TetrisApp);
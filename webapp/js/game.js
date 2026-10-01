(function (app) {
  "use strict";

  const config = app.config;

  class Game {
    constructor() {
      this.field = [];
      this.bag = [];
      this.nextShape = 0;
      this.current = null;
      this.heldShape = null;
      this.holdUsed = false;
      this.score = 0;
      this.lines = 0;
      this.level = 1;
      this.gravityFrame = 0;
      this.clearAnimation = null;
      this.state = "playing";
      this.newRun();
    }

    newRun() {
      this.field = Array.from({ length: config.height }, () => Array(config.width).fill(0));
      this.bag = [];
      this.nextShape = this.takePiece();
      this.heldShape = null;
      this.holdUsed = false;
      this.score = 0;
      this.lines = 0;
      this.level = 1;
      this.gravityFrame = 0;
      this.clearAnimation = null;
      this.state = "playing";
      this.spawnPiece();
    }

    refillBag() {
      const freshBag = [0, 1, 2, 3, 4, 5, 6];
      for (let index = freshBag.length - 1; index > 0; index -= 1) {
        const swapIndex = Math.floor(Math.random() * (index + 1));
        [freshBag[index], freshBag[swapIndex]] = [freshBag[swapIndex], freshBag[index]];
      }
      this.bag.push(...freshBag);
    }

    takePiece() {
      if (this.bag.length === 0) this.refillBag();
      return this.bag.shift();
    }

    canPlace(piece) {
      if (!piece) return false;
      return this.canPlaceAt(piece.shape, piece.rotation, piece.x, piece.y);
    }

    canPlaceAt(shape, rotation, originX, originY) {
      if (shape < 0 || shape >= config.shapes.length || rotation < 0 || rotation >= 4) return false;
      const matrix = config.shapes[shape][rotation];
      for (let row = 0; row < 4; row += 1) {
        for (let column = 0; column < 4; column += 1) {
          if (matrix[row][column] !== "#") continue;
          const boardX = originX + column;
          const boardY = originY + row;
          if (boardX < 0 || boardX >= config.width || boardY < 0 || boardY >= config.height) return false;
          if (this.field[boardY][boardX] !== 0) return false;
        }
      }
      return true;
    }

    spawnPiece() {
      this.current = { shape: this.nextShape, rotation: 0, x: Math.floor(config.width / 2) - 2, y: 0 };
      this.nextShape = this.takePiece();
      this.holdUsed = false;
      this.gravityFrame = 0;
      if (!this.canPlace(this.current)) this.finishRun();
    }

    movePiece(offsetX, offsetY, countSoftDrop = false) {
      if (this.state !== "playing" || this.clearAnimation || !this.current) return false;
      const candidate = { ...this.current, x: this.current.x + offsetX, y: this.current.y + offsetY };
      if (!this.canPlace(candidate)) return false;
      this.current = candidate;
      if (countSoftDrop && offsetY > 0) this.score += offsetY;
      return true;
    }

    rotatePiece(direction) {
      if (this.state !== "playing" || this.clearAnimation || !this.current) return false;
      const rotation = (this.current.rotation + direction + 4) % 4;
      const matrix = config.shapes[this.current.shape][rotation];
      let firstOccupiedRow = 0;
      while (firstOccupiedRow < 4 && !matrix[firstOccupiedRow].includes("#")) firstOccupiedRow += 1;
      const firstVisibleOrigin = 2 - firstOccupiedRow;
      const kicks = this.current.shape === 0 ? config.kicks.line : config.kicks.standard;
      for (const [offsetX, offsetY] of kicks) {
        const candidate = { ...this.current, rotation, x: this.current.x + offsetX, y: this.current.y + offsetY };
        if (!this.canPlace(candidate)) continue;
        if (candidate.y < firstVisibleOrigin) {
          const visibleCandidate = { ...candidate, y: firstVisibleOrigin };
          if (this.canPlace(visibleCandidate)) {
            this.current = visibleCandidate;
            return true;
          }
          continue;
        }
        this.current = candidate;
        return true;
      }
      return false;
    }

    hardDrop() {
      if (this.state !== "playing" || this.clearAnimation || !this.current) return false;
      let landingY = this.current.y;
      while (this.canPlaceAt(this.current.shape, this.current.rotation, this.current.x, landingY + 1)) landingY += 1;
      const distance = landingY - this.current.y;
      this.current.y = landingY;
      this.score += distance * 2;
      this.lockCurrent();
      return true;
    }

    holdPiece() {
      if (this.state !== "playing" || this.clearAnimation || !this.current || this.holdUsed) return false;
      const outgoing = this.current.shape;
      if (this.heldShape === null) {
        this.heldShape = outgoing;
        this.spawnPiece();
      } else {
        this.current = { shape: this.heldShape, rotation: 0, x: Math.floor(config.width / 2) - 2, y: 0 };
        this.heldShape = outgoing;
        if (!this.canPlace(this.current)) this.finishRun();
      }
      this.holdUsed = true;
      return true;
    }

    lockCurrent() {
      if (!this.current) return;
      const lockedShape = this.current.shape + 1;
      const matrix = config.shapes[this.current.shape][this.current.rotation];
      for (let row = 0; row < 4; row += 1) {
        for (let column = 0; column < 4; column += 1) {
          if (matrix[row][column] === "#") this.field[this.current.y + row][this.current.x + column] = lockedShape;
        }
      }
      this.current = null;
      const fullRows = [];
      for (let row = 0; row < config.height; row += 1) {
        if (this.field[row].every((cell) => cell !== 0)) fullRows.push(row);
      }
      if (fullRows.length) this.clearAnimation = { rows: fullRows, frame: 0 };
      else this.spawnPiece();
    }

    finishLineClear() {
      const clearedRows = new Set(this.clearAnimation.rows);
      this.field = this.field.filter((_, row) => !clearedRows.has(row));
      for (let count = 0; count < clearedRows.size; count += 1) this.field.unshift(Array(config.width).fill(0));
      const scoring = [0, 100, 300, 500, 800];
      this.score += (scoring[clearedRows.size] || clearedRows.size * 100) * this.level;
      this.lines += clearedRows.size;
      this.level = Math.floor(this.score / 500) + 1;
      this.clearAnimation = null;
      this.spawnPiece();
    }

    finishRun() {
      if (this.state === "gameover") return;
      this.state = "gameover";
      this.current = null;
    }

    gravityDelay() {
      return Math.max(100, 500 - (this.level - 1) * 40);
    }

    step() {
      if (this.state !== "playing") return false;
      if (this.clearAnimation) {
        this.clearAnimation.frame += 1;
        if (this.clearAnimation.frame >= 12) this.finishLineClear();
        return true;
      }
      this.gravityFrame += config.tickMs;
      if (this.gravityFrame < this.gravityDelay()) return false;
      this.gravityFrame = 0;
      if (this.current && this.canPlace({ ...this.current, y: this.current.y + 1 })) this.current.y += 1;
      else this.lockCurrent();
      return true;
    }
  }

  app.Game = Game;
})(window.TetrisApp);

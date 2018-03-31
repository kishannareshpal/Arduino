var five = require("johnny-five"),
    board = new five.Board();
    // Parallel LCD

board.on("ready", function() {
  console.log('ready, set and free!!');

});

const Five = require("johnny-five");
const Keypress = require("keypress");
const Helper = require("./helpers");
const CP = require ("child_process");
var board, lcd;

board = new Five.Board();

board.on("ready", function() {

  // Vars
  var col, row;
  var buzzerPin = 8;

  // Vars initialization
  col = -1;
  row = 0;


  // setup
    Keypress(process.stdin); // make `process.stdin` begin emitting "keypress" events
    this.pinMode(buzzerPin, Five.Pin.OUTPUT); // set buzzer pinMode


    lcd = new Five.LCD({
      // LCD pin name  RS  EN  DB4 DB5 DB6 DB7
      // Arduino pin # 7    8   9   10  11  12
      pins: [4, 6, 10, 11, 12, 13],
      backlight: 6,
      rows: 2,
      cols: 20,
      // Options:
      // bitMode: 4 or 8, defaults to 4
      // lines: number of lines, defaults to 2
      // dots: matrix dimensions, defaults to "5x8"
    });


    lcd.autoscroll();

    var isHigh = false;

  // Events : Keypress event
  process.stdin.on('keypress', function (ch, key) {    
    console.log(ch, "CH");
    console.log(key, "KEY");    
    
    if (key != null){
      if (key.name == "space"){

        setInterval(() => {
          if (isHigh){
            board.digitalWrite(buzzerPin, 0);
            isHigh = false;
          } else {
            board.digitalWrite(buzzerPin, 1);
            isHigh = true;
          }
        }, 300);
        
      }

      if (key.name == "left"){
        col = col!=0 ? col-1 : 0;
        lcd.cursor(row, col);
        return;
      }
      
      if (key.name == "right") {
        col = col==-1 ? col=1 : col!=15 ? col+1 : 0 ;
        lcd.cursor(row, col);
        return;
      }
      
      if (key.name == "up"){
        row = row == 1 ? 0 : row;
        lcd.cursor(row, col);
        return;
      }

      if (key.name == "down"){
        row = row == 0 ? 1 : row;
        lcd.cursor(row, col);
        return;
      }

      if (key.name == "backspace"){
        row = 0;
        col = -1;
        lcd.cursor(row, col);
        lcd.clear();
        console.log("backspace ran");
        return;
      }

      if (key.name == "return") {
        row = row == 0 ? 1 : 0;
        lcd.cursor(row, col);
        console.log("return ran");
        return;
      }
    }




    if (col != 15) {
      col = col + 1;
      console.log("+1");

    } else {
      console.log("else")
      if (col == 15 && row == 0){
        col = 0;
        row = 1;
        console.log("else 1");
      } else if (col == 15 && row == 1){
        lcd.clear();
        col = 0;
        row = 0;
        console.log("else 2");
      }
    }

    console.log("printed because it reached this statement end.");
    lcd.cursor(row, col);
    lcd.print(ch);

  });

  process.stdin.setRawMode(true);
  process.stdin.resume();

  this.repl.inject({
    lcd: lcd
  });
});
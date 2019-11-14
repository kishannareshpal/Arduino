const Five = require("johnny-five");
const board = new Five.Board();
const Keypress = require("keypress");

// PINS
var motor1 = 11;
var motor2 = 10;

board.on("ready", function() {
  this.pinMode(motor1, Five.Pin.PWM);
  this.pinMode(motor2, Five.Pin.PWM);
  Keypress(process.stdin); // make `process.stdin` begin emitting "keypress" events


  process.stdin.on('keypress', function (ch, key) {
    if (key.name == "right"){
      goRight(255);
      console.log("clock");
      
    } else if (key.name == "left") {
      goLeft(255);
      console.log("stop");
    
    } else {
      stop();
    }

  });

  process.stdin.setRawMode(true);
  process.stdin.resume();
});



function goLeft(speed){ // should do clockwise.
  board.analogWrite(motor1, speed); //set the speed of motor
  board.analogWrite(motor2, 0);     //stop the motorIn2 pin of motor
}

function goRight(speed){
  board.analogWrite(motor1, 0);
  board.analogWrite(motor2, speed);  
}


function stop(){
  board.analogWrite(motor1, 0);
  board.analogWrite(motor2, 0);
}
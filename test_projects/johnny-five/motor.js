var five = require("johnny-five"),
  board = new five.Board();

var m1 = 11, m2 = 10;



board.on("ready", function() {
  this.pinMode(m1, five.Pin.OUTPUT); // set buzzer pinMode  
  this.pinMode(m2, five.Pin.OUTPUT); // set buzzer pinMode  

  clockwise(0);
});

function clockwise(speed){
  board.analogWrite(m1, speed); //set the speed of motor
  board.analogWrite(m2, 0);     //stop the motorIn2 pin of motor
}
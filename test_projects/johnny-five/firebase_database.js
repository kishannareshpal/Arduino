var firebase = require("firebase");

var five = require("johnny-five"),
  board, button;

board = new five.Board();

var app = firebase.initializeApp({
    apiKey: "",
    authDomain: "",
    databaseURL: "",
    projectId: ""
});

var fbdatabase_ = firebase.database();

board.on("ready", function() {

  // Create a new `button` hardware instance.
  // This example allows the button module to
  // create a completely default instance
  button = new five.Button(2);

  // Inject the `button` hardware into
  // the Repl instance's context;
  // allows direct command line access
  board.repl.inject({
    button: button
  });

  // Button Event API
  var i = 0;
  // "down" the button is pressed
  button.on("down", function() {
    i++;
    var fbb = fbdatabase_.ref('cliques').set({
      "times": i
    });

    fbb.catch((error) => {
      console.log(error);
    });
  });

  // "hold" the button is pressed for specified time.
  //        defaults to 500ms (1/2 second)
  //        set
  button.on("hold", function() {
    i++;

    var fbb = fbdatabase_.ref('cliques').set({
      "Stelio": i
    });

    fbb.catch((error) => {
      console.log(error);
    });

  });

  // "up" the button is released
  // button.on("up", function() {
  //   console.log("up");
  // });
});

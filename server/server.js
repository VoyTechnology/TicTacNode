// Include neccessary packages
var http = require('http'),
    url = require('url');

// Declare the return object. Global as functions use it
var ret = {};

// The game "database". Stores all games, users, boards and moves.
// DATA LOST ON RESTART
var game = {
  // The total amount of players playing the game
  players: 0,

  // Current games
  games: []
};

// Create the server
http.createServer( function(req , res){
  // Respond with HTTP status 200 OK, and set the content-type to json
  res.writeHead(200, {'Content-Type': 'application/json'});

  // Parse the URL to get the GET requests
  var get = url.parse(req.url, true);

  // Declare the query and pathname
  var q = get.query,
      p = get.pathname;

  // Check is the action given
  if(p === "/"){
    err(103);

  // Check are any parameters specified
  } else if(Object.getOwnPropertyNames(q).length === 0){
    err(101);

  // If everything is correct, proceed.
  } else {
    switch(p){
      // Do this on new Game
      case '/newGame':
        newGame(q);
        break;

      // Do this on next
      case '/next':
        next(q);
        break;

      // Do this on move
      case '/move':
        move(q);
        break;

      // If the request is not found, return Error 103
      default:
        err(103);
        break;
    }
  }


  // Return the finished JSON result and clear the result
  res.end( JSON.stringify(ret) );
  ret = {};

// Set the server port
}).listen(1337);










////////////////////////////////////
//    ALL NECCESSARY FUNCTIONS    //
////////////////////////////////////




// MAIN FUNCTIONS //


// newGame function
function newGame(q){

  // Check is the passed request "name"
  if(!checkParam("name", q)){
    err(102);

  // If the passed parameter is name, but its empty, return error 101
  } else if(q.name.length === 0){
    err(101);

  // If everything is fine, continue
  } else {

    // Join the game, or make a new one if nowhere to join
    var gm = joinGame(q);

    // Return data
    ret.status = "okay";
    ret.id = gm.id;
    ret.letter = gm.letter;
  }
}

// next function
function next(q){

  // Check is the passed request "id"
  if(!checkParam("id", q)){
    err(102);

  // If the passed parameter is id, but its empty, return error 101
  } else if(q.id.length === 0){
    err(101);

  // If everything is right, continue
  } else {

    // Get the game status
    var gs = gameStatus(q);

    // If game id is was not found, output error 100
    if(gs == -1){
      err(100);

    // If everything is fine, continue
    } else {
      ret.status = "okay";
      ret.board = gs.board;

      // If there is a winner, output that
      if(gs.winner != -1){
        ret.winner = gs.winner;
      } else {
        ret.turn = gs.turn;
      }
    }


  }
}


// Move function
function move(q){

  // Check are there 2 parameters
  if(Object.getOwnPropertyNames(q).length !== 2){
    err(101);

  // If 2 parameters are given, continue
  } else {

    // Check are the 2 parameters id and position
    if(!checkParam("id", q) || !checkParam("position", q)){
      err(102);

    // Check are is any parameter empty
    } else if(q.id.length === 0 || q.position.length === 0){
      err(101);

    // If everything is right, continue
    } else {
      var dm = doMove(q);

      if(dm == -1){
        err(100);
      } else {
        
      }
    }
  }
}










// SECONDARY "WORKER" FUNCTIONS //

// Function make for joining games
function joinGame(q){

  // Counter
  var i = 0;

  //Make a game object
  var gameObj = {

    // Store each player
    player: [
      {
        name: "",
        id: null,
      },
      {
        name: "",
        id: null,
      }
    ],

    // The game board
    board: [0,0,0,0,0,0,0,0,0],

    // Who is the next player do to the movement
    next: 1
  };

  // If games exists, check does user exist
  if(game.games.length > 0){

    // Cycle through all current games
    for(i = 0; i < game.games.length; i++){

      // Make the g the current game
      var g = game.games[i];

      // Check is the player already in the game (for x)
      if(g.player[0].name === q.name){
        return { id: "game-"+g.player[0].id, letter: 1 };

      // Check is the player already in the game (for x)
      } else if(g.player[1].name === q.name){
        return { id: "game-"+g.player[1].id, letter: 2 };

      // Check is the second player empty. If it is, play as it
      } else if(g.player[1].name === ""){
        g.player[1].name = q.name;
        g.player[1].id = game.players++;

        return { id: "game-"+g.player[1].id, letter: 2 };
      }

    }
  }


  // If no places are available to fill, make a new game
  gameObj.player[0].name = q.name;
  gameObj.player[0].id = game.players++;


  // Add the new player to the game
  game.games.push(gameObj);

  // Return the new data
  return {id: "game-"+gameObj.player[0].id, letter: 1};

}



// Get the game of the player
function getPlayerGame(q){

  //Cycle through the games to find the player
  for(var i = 0; i < game.games.length; i++){
    var g = game.games[i];
    var id = q.id.split('-')[1];

    // If player was found, return the game number
    if(g.player[0].id == id || g.player[1].id == id){
      return i;
    }
  }

  // If the player is not found, return -1
  return -1;

}


//Check the status of the game
function gameStatus(q){

  // Get the game of the player
  var g = getPlayerGame(q);

  // If player was not found, return -1
  if(g == -1){
    return -1;
  } else {
    var result = {};
    var cg = game.games[g];

    result.board = cg.board;
    result.turn = cg.next;

    var w = checkWinner();

    if(w != -1){
      result.winner = w;
    } else {
      result.winner = -1;
    }

    return result;

  }
}



// Check who is the winner
function checkWinner(){
  return -1;
}


// Function to check are the request parameters ok
function checkParam(param, q){
  if(Object.getOwnPropertyNames(q).indexOf(param) == -1){
    return false;
  } else {
    return true;
  }
}


// doMove function
function doMove(q){
  var id = getPlayerGame(q);

  if(id == -1){
    return -1;
  } else {

  }
}









// ERROR HANDLING //

// Error handling
function err(c){
  ret.status = "error";
  ret.code = c;
  switch(c){
    case 100:
      ret.message = "Other Error";
      break;

    case 101:
      ret.message = "Missing parameter in request";
      break;

    case 102:
      ret.message = "Unknown parameter in request";
      break;

    case 103:
      ret.message = "Unknown request";
      break;

    case 104:
      ret.message = "Server not ready or busy";
      break;

    case 105:
      ret.message = "Not players turn";
      break;

    case 106:
      ret.message = "Invalid move";
      break;
  }
}

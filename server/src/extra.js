var db = require('./db.js'),
    btoa = require('btoa');

module.exports = {
  // Removes old games,
  removeOldGames: function(s_t){
    db.games.update(
      { "last_move": { $lt: s_t - 3 } },
      { $set: { winner: -1 } },
      function(){}
    );
  },

  // Updates the server time in the database and returns the time
  updateServerTime: function(){
  	db.meta.findAndModify({
  		query: {},
  		update: { $inc: { server_time: 1 } },
  		new: true,
  		upsert: true
  	});
  },

  // Check does the list of requests contain a specific request
  checkParam: function(param, q){
    if(!q.hasOwnProperty(param)){
      return false;
    } else {
      if(q[param].length === 0){
        return false;
      } else {
        return true;
      }
    }
  },

  // Check how many parameters there is
  paramsLength: function(q){
    return Object.getOwnPropertyNames(q).length;
  },

  // Generate the secret
  secretGen: function(){
    var r1 = Math.floor( Math.random()*Math.pow(10,17) ).toString(),
        r2 = Math.floor( Math.random()*Math.pow(10,17) ).toString(),
        r = r1+r2;

        r = r.length%2 === 0 ? r+"ab" : r;

    return btoa( r );
  }
};

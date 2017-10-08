const querystring=require('querystring');
var express = require('express');
var app = express();
var jsonfile = require('jsonfile');

// Our handler function is passed a request and response object
app.get('/newfile', function(req, res) {
  // We must end the request when we are done handling it
  var to = req.query.to,
  from = req.query.from;
  fileName = req.query.fileName;
  description = req.query.description;
  amount = req.query.amount;
  prevHash = req.query.prevHash;
  curHash = req.query.curHash;
  jsonfile.writeFile('transactions.json', "to:" + to + "from:" + from +"fileName:" + fileName +"description:" + description +"amount:" + amount +"prevHash:" + prevHash +"curHash:" + curHash )
  console.log("to:" + to + "from:" + from +"fileName:" + fileName +"description:" + description +"amount:" + amount +"prevHash:" + prevHash +"curHash:" + curHash );
  res.end("OK");
});
app.get('/buyfile', function(req, res) {
  // We must end the request when we are done handling it
  var to = req.query.to,
  from = req.query.from;
  fileName = req.query.fileName;
  description = req.query.description;
  amount = req.query.amount;
  prevHash = req.query.prevHash;
  curHash = req.query.curHash;
  jsonfile.writeFile('transactions.json', "to:" + to + "from:" + from +"fileName:" + fileName +"description:" + description +"amount:" + amount +"prevHash:" + prevHash +"curHash:" + curHash )
  console.log("to:" + to + "from:" + from +"fileName:" + fileName +"description:" + description +"amount:" + amount +"prevHash:" + prevHash +"curHash:" + curHash );
  res.end("OK");
});


module.exports = app;
app.listen(80);
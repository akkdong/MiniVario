const express = require('express')
const bodyParser = require('body-parser');
const fs = require('fs');

const app = express()
const port = 8080

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json())

app.post('/update/:config', (req, res) => {
	console.log("update: ", req.params.config);
	console.log(JSON.stringify(req.body, null, '\t'));
	
	fs.writeFile("test-" + req.params.config, JSON.stringify(req.body, null, '\t'), (err) => {
		if (err) {
			res.status(400).send("FAILED!");
		} else {
			res.status(200).send("OK");
		}
	});
});

/* serves all the static files */
app.get(/^(.+)$/, function(req, res){ 
	console.log('static file request : ' + JSON.stringify(req.params));
	res.sendFile( __dirname + req.params[0]); 
});

app.listen(port, () => {
	console.log(`Simple Web-Server listening on port ${port}!`);
});

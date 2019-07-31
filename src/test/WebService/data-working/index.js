const express = require('express')
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');

const app = express()
const port = 8080

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json())

app.post('/Update/:config', (req, res) => {
	console.log("Update: ", req.params.config);
	console.log(JSON.stringify(req.body, null, '\t'));
	
	fs.writeFile("test-" + req.params.config, JSON.stringify(req.body, null, '\t'), (err) => {
		if (err) {
			res.status(400).send("FAILED!");
		} else {
			res.status(200).send("OK");
		}
	});
});

app.get('/TrackLogs/list', (req, res) => {
	const dir = path.join(__dirname, 'TrackLogs');
	var list = [];

	console.log('TrackLogs List...');
	fs.readdir(dir, function (err, files) {
		if (err) {
			console.log('Unable to scan directory: ' + err);
			res.status(440).end();
		} else {
			console.log(files);
			files.forEach(function (file) {
				if (file.slice(-4).toLowerCase() === '.igc') {
					try {
						var full = path.join(dir, file);
						var stat = fs.statSync(full)
						console.log(file, stat);

						if (stat.isFile && stat.size > 0) {
							list.push({name: file, date: stat.mtime, size: stat.size});
						}
					} catch (e) {
						// ...
					}
				}
			});
			
			res.status(200).set('Content-Type', 'application/json').send(JSON.stringify(list));
		}
	});	
});

app.get('/TrackLogs/:file', (req, res) => {
	console.log('Download TrackLogs : ', req.params.file);
	var dir = path.join(__dirname, 'TrackLogs');
	res.sendFile(path.join(dir, req.params.file));
});

app.delete('/TrackLogs/:file', (req, res) => {
	console.log('Delete TrackLogs : ', req.params.file);
	var dir = path.join(__dirname, 'TrackLogs');
	fs.unlink(path.join(dir, req.params.file), (err) => {
		if (err) {
			res.status(404).send(JSON.stringify(err.statusText));		
			console.log(err);
		} else {
			res.status(200).send("OK");
		}
	});
});

/* serves all the static files */
app.get(/^(.+)$/, function(req, res){ 
	console.log('File Request : ' + JSON.stringify(req.params));
	res.sendFile( __dirname + req.params[0]); 
});

app.listen(port, () => {
	console.log(`Simple Web-Server listening on port ${port}!`);
});

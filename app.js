const express = require('express')
const app = express()
const port = 50004
const RecordManager = require('./recordManager')
const bodyParser = require('body-parser')
const FileUtils = require('./utils/file')
app.use(bodyParser.json());

app.use('/streamvideos', express.static('output'))

let host = process.env.DOMAIN_VIDEO | 'http://localhost:50004/streamvideos'


app.get('/', (req, res) => {
    res.json({
        message: "hello recording service"
    })
})
app.get('/videos/:sid', async (req, res) => {
    let { sid } = req.params
    let urlFile = await FileUtils.getFileHasFormatMP4(sid);
    res.json({
        url: `${host}/${sid}/${urlFile}`
    })
})
app.post('/recorder/v1/start', (req, res, next) => {
    let { body } = req;
    let { appid, channel, key } = body;
    if (!appid) {
        throw new Error("appid is mandatory");
    }
    if (!channel) {
        throw new Error("channel is mandatory");
    }
    RecordManager.start(key, appid, channel).then(recorder => {
        //start recorder success
        res.status(200).json({
            success: true,
            sid: recorder.sid
        });
    }).catch((e) => {
        console.log(e)
        res.json({
            message: "Recorder not found"
        })
        //start recorder failed
    });
})

app.post('/recorder/v1/stop', (req, res, next) => {
    let { body } = req;
    let { sid } = body;
    if (!sid) {
        throw new Error("sid is mandatory");
    }

    RecordManager.stop(sid);
    res.status(200).json({
        success: true
    });
})
app.use((err, req, res, next) => {
    console.error(err.stack)
    res.status(500).json({
        success: false,
        err: err.message || 'generic error'
    })
})

app.listen(port, () => {
    console.log("Listening port: " + port)
})
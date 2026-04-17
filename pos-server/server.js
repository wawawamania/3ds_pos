const express = require('express');
const app = express();
const port = 8000;

app.use(express.json());

const items = [
    { id: 1, name: 'コーラ', price: 120 },
    { id: 2, name: 'ポテト', price: 280 },
    { id: 3, name: 'バーガー', price: 350 }
];

app.get('/status',(req, res) => {
    res.json({
        ok: true,
        message: '3DS POS server alive'
    });
});

app.get('/items',(req,res) => {
    res.set('Connection','close');
    res.json({
        ok:true,
        message: '3DS POS server alive'
    });
});

app.post('/sale',(req, res) => {
    const body = req.body;
    console.log('sale received:' ,body);

    res.json({
        ok: true,
        received: body
    });
});

app.listen(port, '0.0.0.0',() => {
    console.log(`POS server running: http://192.168.11.63:${port}`)
})
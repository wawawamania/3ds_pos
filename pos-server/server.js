const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 8000;

app.use(express.json());

const items = [
    { id: 1, name: 'コーラ', price: 120 },
    { id: 2, name: 'ポテト', price: 280 },
    { id: 3, name: 'バーガー', price: 350 }
];

const dbPath = path.join(__dirname, 'orders.json')

function readOrders(){
    try {
        const text = fs.readFileSync(dbPath,'utf-8');
        return JSON.parse(text);
    } catch (err) {
        return [];
    }
}

function writeOrders(orders){
    fs.writeFileSync(dbPath,JSON.stringify(orders,null, 2), 'utf-8')
}

app.get('/status',(req, res) => {
    res.set('Connection','close')
    res.json({
        ok: true,
        message: '3DS POS server alive'
    });
});

app.get('/items', (req, res) => {
    res.set('Connection', 'close');
    res.json({ items });
});

app.post('/orders',(req, res) => {
    res.set('Connection', 'close');

    const body = req.body;

    if(!body.productId || !body.productName) {
        return res.status(400).json({
            ok: false,
            message: 'productID and productName are required'
        });
    }

    const orders = readOrders();

    const newOrder = {
        id: orders.length + 1,
        productId: body.productId,
        productName: body.productName,
        qty: body.qty ?? 1,
        orderedAt: new Date().toISOString()
    };

    orders.push(newOrder);
    writeOrders(orders);

    console.log('order received:',newOrder);

    res.json({
        ok:true,
        order:newOrder
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
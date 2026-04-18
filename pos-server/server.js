const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 8000;

const items = [
  { id: 1, name: 'コーラ', price: 120 },
  { id: 2, name: 'ポテト', price: 280 },
  { id: 3, name: 'バーガー', price: 350 }
];

const dbPath = path.join(__dirname, 'orders.json');

function readOrders() {
  try {
    const text = fs.readFileSync(dbPath, 'utf-8');
    return JSON.parse(text);
  } catch (err) {
    return [];
  }
}

function writeOrders(orders) {
  fs.writeFileSync(dbPath, JSON.stringify(orders, null, 2), 'utf-8');
}

app.get('/status', (req, res) => {
  res.set('Connection', 'close');
  res.json({
    ok: true,
    message: '3DS POS server alive'
  });
});

app.get('/items', (req, res) => {
  res.set('Connection', 'close');
  res.json({ items });
});

app.get('/orders', (req, res) => {
  res.set('Connection', 'close');
  const orders = readOrders();
  res.json({ ok: true, orders });
});

// /orders だけ text で受ける
app.post('/orders', (req, res) => {
  res.set('Connection', 'close');

  console.log('query:', req.query);

  const productId = Number(req.query.productId);
  const productName = req.query.productName;
  const qty = Number(req.query.qty ?? 1);

  if (!productId || !productName) {
    return res.status(400).send('bad request');
  }

  const orders = readOrders();

  const newOrder = {
    id: orders.length + 1,
    productId,
    productName,
    qty,
    orderedAt: new Date().toISOString()
  };

  orders.push(newOrder);
  writeOrders(orders);

  console.log('order received:', newOrder);

  res.status(200).send('ok');
});

app.listen(port, '0.0.0.0', () => {
  console.log(`POS server running: http://192.168.11.46:${port}`);
});
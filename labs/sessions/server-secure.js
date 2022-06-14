const express = require('express')
const cookieParser = require('cookie-parser')
const createReadStream = require('fs').createReadStream
//const { createReadStream } = require('fs')
const bodyParser = require('body-parser')
const randomBytes = require('crypto').randomBytes
//const { randomBytes } = require('crypto')

const app = express()
app.use(cookieParser())
app.use(bodyParser.urlencoded({ extended: false }))
// Routes go here!
app.listen(4000)

const USERS = { alice: 'password', bob: '50505' }
const BALANCES = { alice: 500, bob: 100 }
const SESSIONS = {} // sessionId -> username

app.get('/', (req, res) => {
  const sessionId = req.cookies.sessionId
  const username = SESSIONS[sessionId]
  if (username) {
    res.send(`Hi ${username}. Your balance is $${BALANCES[username]}.`) 
  } else {
    createReadStream('index.html').pipe(res)
  }
})
 
app.post('/login', (req, res) => {
  const username = req.body.username
  const password = USERS[username]
  if (password === req.body.password) {
    const sessionId = randomBytes(16).toString('hex')
    SESSIONS[sessionId] = username
    res.cookie('sessionId', sessionId)
    res.redirect('/')
  } else {
    res.send('fail!')
  }
}) 

app.get('/logout', (req, res) => {
  const sessionId = req.cookies.sessionId
  delete SESSIONS[sessionId]
  res.clearCookie('sessionId')
  res.redirect('/')
})

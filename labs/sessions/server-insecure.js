const express = require('express')
const cookieParser = require('cookie-parser')
const createReadStream = require('fs').createReadStream
//const { createReadStream } = require('fs')
const bodyParser = require('body-parser')

const app = express()
app.use(cookieParser())
app.use(bodyParser.urlencoded({ extended: false }))
// Routes go here!
app.listen(4000)

const USERS = { alice: 'password', bob: '50505' }
const BALANCES = { alice: 500, bob: 100 }
// let nextSessionId = 1
nextSessionId = 1
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
    SESSIONS[nextSessionId] = username
    res.cookie('sessionId', nextSessionId)
    nextSessionId += 1
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

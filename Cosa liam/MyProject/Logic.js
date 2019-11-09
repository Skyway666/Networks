



// Input handling
var search_bar = document.getElementById("search_bar")

//enum
var search_criteria = "employee" // "datecreated", "location", "injurytype"
//enum
var ordering = "desc" // "asc"
var finished_requests = 0

function onSearchBar(){

  // Clean arrays

  incidents = []
  injury_types = []
  locations = []
  tableData = []
  

  // Make a request to the API
    
   getIncidents(search_bar.value, search_criteria, ordering)
   // TODO: Optimization- If the arrays are already ordered in a descending way, no need to get them again
   getEmployees()
   getInjuryTypes()
   getLocations()
   getBodyParts()
  
}


// VARIABLES
var incidents = {} // Contains all the id's


var employees = {} // All employees THAT EXIST
var injury_types = {} //All injuries THAT EXIST
var locations = {} // All locations THAT EXIST
var bodyParts = {} // All body parts THAT EXIST

var tableData = []

// REQUESTS
var incidents_request = new XMLHttpRequest() 

var employees_request = new XMLHttpRequest() 
var injurys_request = new XMLHttpRequest() 
var locations_request = new XMLHttpRequest()
var bodyParts_request = new XMLHttpRequest()

incidents_request.onload = function() {
  incidents = JSON.parse(this.response)
  finished_requests += 1
}

employees_request.onload = function() {
  employees = JSON.parse(this.response)
  finished_requests += 1
}
injurys_request.onload = function() {
  injury_types = JSON.parse(this.response)
  finished_requests += 1
}
locations_request.onload = function() {
  locations = JSON.parse(this.response)
  finished_requests += 1
}

bodyParts_request.onload = function(){
  bodyParts = JSON.parse(this.response)
  finished_requests += 1
}


// This functions will fill variables
function getIncidents(filter){
  var request = "http://localhost:3000/api/v1/incident";
  if(filter.length > 0){
  request = "http://localhost:3000/api/v1/incident?sort=employee&search="
  request = request.concat(filter)
  }
  incidents_request.open('GET', request , true)
  incidents_request.send()
}

function getEmployees(){
  employees_request.open('GET', 'http://localhost:3000/api/v1/employee', true)
  employees_request.send()
}
function getInjuryTypes(){
  injurys_request.open('GET', 'http://localhost:3000/api/v1/injuryType', true)
  injurys_request.send()
}
function getLocations(){
  locations_request.open('GET', 'http://localhost:3000/api/v1/location', true)
  locations_request.send()
}

function getBodyParts(){
  bodyParts_request.open('GET', 'http://localhost:3000/api/v1/bodyPart', true)
  bodyParts_request.send()
}


// This functions will use variables to confortably fill the table
function createTableData(){
  incidents.forEach(incident => {
    var data = {}
    data.date = incident.dateCreated
    data.employee = getDataById(employees, incident.employeeId).fullName
    data.injury_type = getDataById(injury_types, incident.injuryTypeId).name
    data.location = getDataById(locations, incident.locationId).name
    data.bodyParts = getDataById(bodyParts, incident.bodyPartIds[0]).name // TODO: Functionality - get all body parts

    tableData.push(data)
    });
}

function getDataById(array, id){
  var ret = null

  array.forEach(instance => {
    if(instance.id == id){
      ret = instance
      // TODO: Optimization - stop loop when we find
    }
  })
  return ret
}
// Loop function to wait for all requests to return

//Start :)

onSearchBar()

function update(){
  if(finished_requests == 5){
    finished_requests = 0
    createTableData()
    //drawTable()
  }
  
  app.tableData = tableData
  window.requestAnimationFrame(update)
}


// What is executed once
window.requestAnimationFrame(update)



// VUE TESTING

var app = new Vue({
  // Vue variables... i guess+
  el: '#table',
  data:{
    tableData: tableData
  }
})


//https://www.taniarascia.com/how-to-connect-to-an-api-with-javascript/ thanks girl
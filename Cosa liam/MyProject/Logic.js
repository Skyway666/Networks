

// Input handling
var search_bar = document.getElementById("search_bar")
var sort_dropdown = document.getElementById("sort")
var sortdir_dropdown = document.getElementById("sortdir")

// Filter and order vars
var filter_criterias =["employee", "datecreated", "location", "injurytype"]
var ordering_criterias = ["desc","asc"] 

// Data from querys vars
var incidents = [] // Contains all the id's

var employees = [] // All employees THAT EXIST
var injury_types = [] //All injuries THAT EXIST
var locations = [] // All locations THAT EXIST
var bodyParts = [] // All body parts THAT EXIST

var tableData = []

// Pagination

var current_page = 1
var total_pages = 0

// VUE 

var tableDynamic = new Vue({
  // Vue variables... i guess+
  el: '#table',
  data:{
    tableData: tableData
  }
})

var filterDynamic = new Vue({
  el: '#search_criteria',
  data:{
    current_page : current_page,
    total_pages: total_pages
  }
})

// REQUESTS

var finished_requests = 0
var incidents_request = new XMLHttpRequest() 

var employees_request = new XMLHttpRequest() 
var injurys_request = new XMLHttpRequest() 
var locations_request = new XMLHttpRequest()
var bodyParts_request = new XMLHttpRequest()

incidents_request.onload = function() {
  incidents = JSON.parse(this.response)
  total_pages = this.getResponseHeader("Page-Count") // Pages for pagination
  filterDynamic.total_pages = total_pages
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


// search_bar -> oninput=

function onNewFilter(){
  current_page = 1
  filterDynamic.current_page = current_page
  makeQuery()
}
function makeQuery(){
  // Make a request to the API
   getIncidents(search_bar.value, filter_criterias[sort_dropdown.selectedIndex], ordering_criterias[sortdir_dropdown.selectedIndex])
}


// This functions will fill variables
function getIncidents(filter, filter_crit, ordering_crit){

  request = "http://localhost:3000/api/v1/incident?search="
  request = request.concat(filter)
  request = request.concat("&sort=" + filter_crit)
  request = request.concat("&sortdir=" + ordering_crit)
  request = request.concat("&page=" + current_page)

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

function changePageForward(){
  changePage("forward")
}
function changePageBackwards(){
  changePage("backwards")
}

function changePage(direction){
  if(direction == "forward" && current_page != total_pages) 
    current_page++
  else if(direction == "backwards" && current_page != 1) 
    current_page--

  filterDynamic.current_page = current_page
  makeQuery();
}


// This functions will use variables to confortably fill the tableData
function createTableData(){

  tableData = []

  incidents.forEach(incident => {
    var data = {}
    data.date = extractDate(incident.dateCreated)
    data.employee = getDataById(employees, incident.employeeId).fullName
    data.injury_type = getDataById(injury_types, incident.injuryTypeId).name
    data.location = getDataById(locations, incident.locationId).name
    
    // First body part
    data.bodyParts = getDataById(bodyParts, incident.bodyPartIds[0]).name
    // Concat all body parts in bodyPartsIds
    for(var i = 1; i < incident.bodyPartIds.length;i++ )
      data.bodyParts = data.bodyParts.concat(", " + getDataById(bodyParts, incident.bodyPartIds[i]).name) 
    
    tableData.push(data)
    });

    tableDynamic.tableData = tableData
}

function getDataById(array, id){
  var ret = null

  array.forEach(instance => {
    if(instance.id == id){
      // TODO: Optimization - stop loop when we find
      ret = instance
    }
  })
  return ret
}

function extractDate(date_string){
  var index = date_string.indexOf("T")
  return date_string.slice(0, index)

}



//Start :)


getEmployees()
getInjuryTypes()
getLocations()  
getBodyParts()
makeQuery()

window.requestAnimationFrame(update)

// Loop function to wait for all requests to return
function update(){
  if(finished_requests == 5){
    finished_requests = 4
    createTableData()
  }

  window.requestAnimationFrame(update)
}

//https://www.taniarascia.com/how-to-connect-to-an-api-with-javascript/ thanks girl
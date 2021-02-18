function logoutOK(response) {
  window.location = "/login";
}
function logoutFAILED(response) {
  console.log("Your sessionId is not working anymore in the remote side");
  window.location = "/login";
}
function logout() {
  $.ajax({
    url: '/api?mode=LOGOUT',
    type: 'POST',
    headers: { "CSRFToken": $("#csrfToken").val().trim() },
    success: logoutOK,
    error: logoutFAILED
  });
}

var intervalId = -1;

function ajaxAuthReCheck() 
{
  $.ajax({
    url: '/api?mode=AUTHINFO',
    headers: { "CSRFToken": $("#csrfToken").val().trim() },
    type: 'POST',
    success: function (response)
    {
      // Reestablish the interval with the new maxAge.
      clearInterval(intervalId);
      var intTime = parseInt(response["maxAge"],10)+1;
      console.log("Setting up the next session check to " + intTime + " sec's in future.");
      intervalId = setInterval(ajaxAuthReCheck, (intTime)*1000);
      console.log("Session renewed: " + response["maxAge"] + " sec's left" );
    },
    error: function (xhr, ajaxOptions, thrownError) {
      if(xhr.status==404) 
      {
        // Session is gone...
        clearInterval(intervalId);
        alert("Session Expired");
        logout();
      }
      else
      {
        console.log("Network error " + xhr.status);
      }
    }
  });
}

function ajaxLoadInfo() {
  $('#welcome').text('Loading...');

  /////////////////////////////////////////////
  // Load the application version...
  $.ajax({
    url: '/api?mode=VERSION',
    type: 'POST',
    success: function (result) {
      $("#version").text(result["version"]);
    }
  });

  /////////////////////////////////////////////////////////////////////
  // Load The CSRF Token and the application welcome message...
  $.ajax({
    url: '/api?mode=CSRFTOKEN',
    type: 'POST',
    success: function (response) {
      if (response) {
        // Set the CSRF Token into the response...
        $('#csrfToken').val(response["csrfToken"]);
        // Load the Authentication Info (username)
        $.ajax({
          url: '/api?mode=AUTHINFO',
          headers: { "CSRFToken": response["csrfToken"] },
          type: 'POST',
          success: function (response) {
            // Set the authentication info...
            $('#welcome').text("Welcome " + response["user"]);

            // Check the authentication when the session is supposed to expire, and logout if there is no session.
            var intTime = parseInt(response["maxAge"],10)+1;
            console.log("Setting up the next session check to " + intTime + " sec's in future.");
            intervalId = setInterval(ajaxAuthReCheck,intTime*1000);
          },
          error: logout
        });
        csrfReady();
      }
    },
    error: logout
  });
}

// Aditionals:


function cActivateView(groupName, id)  {
  // Deactivate all the group name:
  let inputs = document.getElementsByName(groupName);
  for (let input of inputs) 
  {
    if (input.style.display == "block")
    {
      // Deactivate the thing...
    //  console.log("Deactivating " + input.id);
      input.style.display = "none";

      // Deactivate link
      if (document.getElementById(input.id + "-link")) document.getElementById(input.id + "-link").classList.remove("active");

      var a = input.id + '-stop';
      a = a.replace(/-/g, "_");
      if (window[a])
        window[a]();
    }
  }
  // Activate our candidate...
//    console.log("Activating " + id);  
  if (document.getElementById(id)) document.getElementById(id).style.display = "block";
  if (document.getElementById(id + "-link")) document.getElementById(id + "-link").classList.add("active");
  
  var a = id + '-start';
  a = a.replace(/-/g, "_");
  if (window[a]) window[a]();
}
function logoutOK(response) {
    window.location = "/login.html";
  }
  function logoutFAILED(response) {
    console.log("Your sessionId is not working anymore in the remote side");
    window.location = "/login.html";
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
  function AUTHINFOResponseOK(response) {
    $('#welcome').text("Welcome " + response["user"]);
  }
  function CSRFResponseOK(response) {
    if (response) {
      $('#csrfToken').val(response["csrfToken"]);
      $.ajax({
        url: '/api?mode=AUTHINFO',
        headers: { "CSRFToken": response["csrfToken"] },
        type: 'POST',
        success: AUTHINFOResponseOK,
        error: logout
      });
      csrfReady();
    }
  }
  function ajaxLoadInfo() {
    $('#welcome').text('Loading...');
    $.ajax({
      url: '/api?mode=CSRFTOKEN',
      type: 'POST',
      success: CSRFResponseOK,
      error: logout
    });
    $.ajax({
      url: '/api?mode=VERSION',
      type: 'POST',
      success: function(result){
        $("#version").text(result["version"]);
      }
    });
  }
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
function loginSUCCESS(response) {
    $("#message").text("Logged in.");
    window.location = "/main.html";
}
function loginFAILED(request, status, error) {
    $('#message').text(error);
}
function loginCSRF(response) {
    if (response) {
        $("#message").text("Logged in, confirming authentication token.");
        $.ajax({
            url: '/api?mode=AUTHCSRF',
            type: 'POST',
            headers: { "CSRFToken": response["csrfAuthConfirm"] },
            data: { sessionId: response["sessionId"] },
            success: loginSUCCESS,
            error: loginFAILED
        });
    }
    else {
        var msg = "Unknown Error...";
    }
}
function ajaxLogin() {
    var username = $("#username").val().trim();
    var auths = { "0": { pass: $("#password").val().trim() } }
    if (username != "") {
        $('#message').text('logging in...');
        $.ajax({
            url: '/api?mode=LOGIN',
            type: 'POST',
            headers: { "CSRFToken": "00112233445566778899" },
            data: { user: username, auths: JSON.stringify(auths) },
            success: loginCSRF,
            error: loginFAILED
        });
    }
}
function ajaxLoadVersion() {
    $.ajax({
        url: '/api?mode=VERSION',
        type: 'POST',
        success: function (result) {
            $("#version").text(result["version"]);
        }
    });
}

function autoLogoutOnLogin() {
    // As cookie should be httpOnly, javascript can't know if there is a session, so, we try to get the CSRF token, 
    $('#message').text('Uhh.');

    $.ajax({
        url: '/api?mode=CSRFTOKEN',
        type: 'POST',
        success: function (result) {
            // if returns the token, the cookie exist...  and logout response must drop the cookie.
            $.ajax({
                url: '/api?mode=LOGOUT',
                type: 'POST',
                headers: { "CSRFToken": result["csrfToken"] },
                success: function (result)
                {
                    $('#message').text('Ready (Logged Out).');
                },
                error: function (result)
                {
                    document.cookie = "sessionId= ; expires = Thu, 01 Jan 1970 00:00:00 GMT";
                    $('#message').text('Error logging out, please refresh by pressing F5.');
                }
            });
        },
        error: function (result) {
            // If CSRF returns nothing, is because the cookie does not exist, or the cookie does not handle a valid session, or the session has not be fully authenticated, in each case, drop the cookie
            document.cookie = "sessionId= ; expires = Thu, 01 Jan 1970 00:00:00 GMT";
            $('#message').text('Ready.');
        }
    });
}

function ajaxLoginStart()
{
    ajaxLoadVersion();
    autoLogoutOnLogin();
}
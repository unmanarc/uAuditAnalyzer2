var changePassword = false;
var requireNextAuth = false;
var nextPassIDX = 0;
var nextPassDESC = "";

function loginFAILED(request, status, error) {

    // Reset Login/Password Status to reenable it:

    document.getElementsByName('username')[0].readOnly = false;
    document.getElementsByName('password')[0].readOnly = false;
    document.getElementsByName('password')[0].value = "";
    document.getElementsByName('password')[0].placeholder = "********";
    document.getElementsByName('password')[0].autocomplete = "on";
    document.getElementsByName('password')[0].focus();
    document.getElementsByName('login')[0].value = "Login";

    // Reset global status:
    changePassword = false;
    requireNextAuth = false;
    nextPassIDX = 0;
    nextPassDESC = "";

    // Set message:
    console.log(request);
    $('#message').text(`${error}: ${request.responseJSON["txt"]}`);

}

function CSRFResponseOKForLogin(response) {
    // Put the CSRF Token into the 
    if (response) {
        $('#csrfToken').val(response["csrfToken"]);
    }
}

function loginGotoNextFactor()
{
    if (!requireNextAuth) {
        // Go to the next HTML WEB
        if (changePassword == true) {
            $("#message").text("Password Expired.");
            window.location = "/secrets";
        }
        else {
            $("#message").text("Logged in.");
            window.location = "/main.html";
        }
    }
    else {
        // Create the next authentication HTML element...
        $("#message").text("Another Password Required(" + nextPassIDX + "): " + nextPassDESC);
        document.getElementsByName('password')[0].readOnly = false;
        document.getElementsByName('password')[0].placeholder = nextPassDESC;
        document.getElementsByName('password')[0].value = "";
        document.getElementsByName('password')[0].autocomplete = "off";
        document.getElementsByName('password')[0].focus();
        document.getElementsByName('login')[0].value = "Next";
    }
}

function loginConfirmSession(response) {

    // Now the CSRF token is fixed in the session, we can get it using the session ID:
    $.ajax({
        url: '/api?mode=CSRFTOKEN',
        type: 'POST',
        success: CSRFResponseOKForLogin,
        error: loginFAILED
    });

    loginGotoNextFactor();    
}

function loginAnswerProcessor(response) {
    if (response) {

        // Authenticate the authentication token and check 
        // First Password (Open Login)
        if (response["val"] == 100) changePassword = true;
        // Set user/pass as readonly.
        document.getElementsByName('username')[0].readOnly = true;
        document.getElementsByName('password')[0].readOnly = true;

        // Set Message to Confirming authentication token.
        $("#message").text("Logged in, confirming authentication token.");

        var lNextPassIDX = nextPassIDX;

        // Check if another pass is required
        if (response["nextPassReq"] != false) {
            requireNextAuth = true;
            nextPassIDX = response["nextPassReq"]["idx"];
            nextPassDESC = response["nextPassReq"]["desc"];
        }
        else
            requireNextAuth = false;

        if (lNextPassIDX == 0) {
            // First time here.
            $.ajax({
                url: '/api?mode=AUTHCSRF',
                type: 'POST',
                headers: { "CSRFToken": response["csrfAuthConfirm"] },
                data: { sessionId: response["sessionId"] },
                success: loginConfirmSession,
                error: loginFAILED
            });
        }
        else {
            // Another password?
            loginGotoNextFactor();
        }

    }
    else {
        $("#message").text("Unknown error.");
    }
}

function ajaxLogin() {

    if (nextPassIDX == 0) {
        // Login for master key ID=0 (Session Openning)
        $('#message').text('logging in...');
        $.ajax({
            url: '/api?mode=LOGIN',
            type: 'POST',
            headers: { "CSRFToken": "00112233445566778899" },
            data: {
                user: $("#username").val(),
                auth: JSON.stringify({ pass: $("#password").val(), idx: 0 })
            },
            success: loginAnswerProcessor,
            error: loginFAILED
        });
    }
    else {
        // POSTLOGIN authentications...
        $('#message').text('Submitting authentication ID(#' + nextPassIDX + ')');

        $.ajax({
            url: '/api?mode=POSTLOGIN',
            type: 'POST',
            headers: { "CSRFToken": $("#csrfToken").val().trim() },
            data: {
                auth: JSON.stringify({ pass: $("#password").val(), idx: nextPassIDX })
            },
            success: loginAnswerProcessor,
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

function ajaxLogout()
{
    document.getElementById("loginform").style.visibility = "visible";
    document.getElementById("logoutform").style.visibility = "collapse";

    $.ajax({
        url: '/api?mode=LOGOUT',
        type: 'POST',
        headers: { "CSRFToken": $("#csrfToken").val().trim() },
        success: function (result) {
            $('#message').text('You have been logged out, Ready.');
        },
        error: function (result) {
            document.cookie = "sessionId= ; expires = Thu, 01 Jan 1970 00:00:00 GMT";
            $('#message').text('Error logging out, please refresh by pressing F5.');
        }
    });
}

function showLogoutDialogOnSession() {
    // As cookie should be httpOnly, javascript can't know if there is a session, so, we try to get the CSRF token, 
//    $('#message').text('Uhh.');
    document.getElementById("loginform").style.visibility = "visible";
    document.getElementById("logoutform").style.visibility = "collapse";

    $.ajax({
        url: '/api?mode=CSRFTOKEN',
        type: 'POST',
        success: function (result) {
            // if returns the token, the cookie exist...  go to logout page.
            document.getElementById("loginform").style.visibility = "collapse";
            document.getElementById("logoutform").style.visibility = "visible";

            $('#message').text('There is an active session.');

            if (result) {
                $('#csrfToken').val(result["csrfToken"]);
            }
        },
        error: function (result) {
            // If CSRF returns nothing, is because the cookie does not exist, or the cookie does not handle a valid session, or the session has not be fully authenticated, in each case, drop the cookie
            document.cookie = "sessionId= ; expires = Thu, 01 Jan 1970 00:00:00 GMT";
            $('#message').text('Ready.');
        }
    });
}

function ajaxLoginStart() {
    ajaxLoadVersion();
    showLogoutDialogOnSession();
}
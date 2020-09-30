function percentageToDegrees(percentage) {  return percentage / 100 * 360 }   

function redrawProgressBars()
{

    $(".progress").each(function() {
      
        var left = $(this).find('.progress-left .progress-bar');
        var right = $(this).find('.progress-right .progress-bar');
        var value = $(this).attr('data-value');
      
        if (value > 0) {
          if (value <= 50) {
            right.css('transform', 'rotate(' + percentageToDegrees(value) + 'deg)')
          } else {
            right.css('transform', 'rotate(180deg)')
            left.css('transform', 'rotate(' + percentageToDegrees(value - 50) + 'deg)')
          }
        }
      
      })
}

function fillPercentage( a1,a2, p1,p2, percentage )
{
    $('#'+ a1).text(percentage);
    document.getElementById(a2).setAttribute("data-value", percentage);

    document.getElementById(p1).classList.remove("border-success");
    document.getElementById(p1).classList.remove("border-warning");
    document.getElementById(p1).classList.remove("border-danger");

    document.getElementById(p2).classList.remove("border-success");
    document.getElementById(p2).classList.remove("border-warning");
    document.getElementById(p2).classList.remove("border-danger");

   if (percentage>75)
    {
      document.getElementById(p1).classList.add("border-danger");
      document.getElementById(p2).classList.add("border-danger");
    }
    else if (percentage>50)
    {
      document.getElementById(p1).classList.add("border-warning");
      document.getElementById(p2).classList.add("border-warning");
    }
    else
    {
      document.getElementById(p1).classList.add("border-success");
      document.getElementById(p2).classList.add("border-success");
    }
}
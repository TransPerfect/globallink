(function($) {
  Drupal.behaviors.transperfect = {
    attach: function (context, settings) {
      // Check to make sure that we're working with the right form.
      if ($('#transperfect-node-active-select-form', context).length == 0) {
        return;
      }

      // Only run on page load.
      if (context != '#document') {
        return;
      }

      // Binds change event on select element and submits form.
      $('#edit-submission', context).change(function() {
        $('#transperfect-node-active-select-form').submit();
      });
    }
  };
})(jQuery);

(function($) {
  Drupal.behaviors.transperfect = {
    attach: function (context, settings) {
      if ($('#transperfect-node-active-select-form', context).length == 0) {
        return;
      }

      if (context != '#document') {
        return;
      }

      $('#edit-submission', context).change(function() {
        $('#transperfect-node-active-select-form').submit();
      });
    }
  };
})(jQuery);

(function($) {
  Drupal.behaviors.transperfect = {
    attach: function (context, settings) {
      if ($('.transperfect-entity-form-clear', context).length == 0) {
        return;
      }

      $('.transperfect-entity-form-clear', context).submit(function() {
        if (!confirm('Are you sure you want to clear the changed status for the selected content(s)?')) {
          return false;
        }
      });
    }
  };
})(jQuery);

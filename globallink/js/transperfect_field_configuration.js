(function($) {
  Drupal.behaviors.transperfectFieldConfiguration = {
    attach: function(context, settings) {
      if ($('.transperfect-field', context).length == 0) {
        return;
      }

      $('.transperfect-field-select-type', context).change(function() {
        $('.transperfect-field').submit();
      });
    }
  };
})(jQuery);

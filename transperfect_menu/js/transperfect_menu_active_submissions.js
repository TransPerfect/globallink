(function($) {
  Drupal.behaviors.transperfectMenuActiveSubmissions = {
    attach: function(context, settings) {
      if ($('.transperfect-menu-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-menu-active-select-form-edit', context).change(function() {
        $('.transperfect-menu-active-select-form').submit();
      });
    }
  };
})(jQuery);

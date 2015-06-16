<?php

if (!class_exists("login")) {

    class login {

        public $username; // string
        public $password; // string

    }

}

if (!class_exists("loginResponse")) {

    class loginResponse {

        public $return; // string

    }

}

if (!class_exists("logout")) {

    class logout {

        public $userId; // string

    }

}

if (!class_exists("logoutResponse")) {

    class logoutResponse {

        public $return; // string

    }

}

/**
 * SessionService2 class
 *
 *
 *
 * @author    {author}
 * @copyright {copyright}
 * @package   {package}
 */
class SessionService2 extends SoapClient {

    private static $classmap = array(
        'login' => 'login',
        'loginResponse' => 'loginResponse',
        'logout' => 'logout',
        'logoutResponse' => 'logoutResponse',
    );

    public function SessionService2($wsdl = "http://localhost:8080/pd4/services/SessionService2?wsdl", $options = array()) {
        foreach (self::$classmap as $key => $value) {
            if (!isset($options['classmap'][$key])) {
                $options['classmap'][$key] = $value;
            }
        }
        parent::__construct($wsdl, $options);
    }

    /**
     *  
     *
     * @param logout $parameters
     * @return logoutResponse
     */
    public function logout(logout $parameters) {
        return $this->__soapCall('logout', array($parameters), array(
            'uri' => 'http://impl.services2.service.ws.projectdirector.gs4tr.org',
            'soapaction' => ''
                )
        );
    }

    /**
     *  
     *
     * @param login $parameters
     * @return loginResponse
     */
    public function login(login $parameters) {
        return $this->__soapCall('login', array($parameters), array(
            'uri' => 'http://impl.services2.service.ws.projectdirector.gs4tr.org',
            'soapaction' => ''
                )
        );
    }

}

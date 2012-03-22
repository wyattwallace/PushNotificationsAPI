<?php


interface IPushableObserved
{
    /**
    * Method for adding observer objects 
    *
    * @param object $observer
    * @access public
    * @abstract
    */
    public function addObserver( IPushableObserver $observer );

    /**
    * This is the method that notifies the attached/added observer objects of the state change
    *
    * @param array $payload
    * @access public
    * @abstract
    */
    public function push( array $payload );
}

interface IPushableObserver
{
    /**
    * All Observer Objects must implement this method.  Observered Object will notify Observers
    * through this method.
    *
    * @param object $sender
    * @access public
    * @abstract
    */
    public function send( IPushableObserved $sender );
}

abstract class AbstractPushableObject implements IPushableObserver
{

    public function sendNotification( $queueName, array $data )
    {

        // send into queue fQueue or mybPublish ?
        fQueue::getInstance( )->enqueue( $queueName, $data );
       
    }

}

class PushableObjectsList implements IPushableObserved
{
    public $err = array();
    
    private $payload = array();
    private $observers = array();

    public function push( array $payload )
    {
        $returnVal = true;

        // validate $payload here
        $this->payload = $payload;

        foreach( $this->observers as $obs ) 
        {
            if ( ! $obs->send( $this ) ) 
            {
                $this->err = array_merge($this->err, $obs->err);
                $returnVal = false;
            }   
        }

        return $returnVal;
    }

    public function addObserver( IPushableObserver $observer )
    {
        $this->observers[] = $observer;
        return true;
    }

    public function getPayload()
    {
        return $this->payload;
    }
}

class PushNotificationDecorator
{
    $pushNotificationObject;

    public function __construct( $deviceType, $payload )
    {
        // if class exists
        $className = $this-> getPushNotificationClassName( $deviceType, $payload['routingKey'] );
        $this->pushNotificationObject = new $className();
    }

    public function getData( $payload )
    {
        return $this->pushNotificationObject->getData( $payload );
    }

    // we need to map the routing key to Push Notification type
    protected function getPushNotificationClassName( $deviceType, $routingKey )
    {
        // string munge here

       // combine to form class name - PushNotificationNewProfileViewAndroid
       $string = 'PushNotification' . $routingKey . $deviceType;

        return $string;
    }

}

class PushableDeviceIphone extends AbstractPushableObject
{
    // possible late static binding, might have to do protected $type = 'iOS';
    const TYPE = 'iOS';
    const QUEUE_NAME = 'iphone_queue_name';
    public $er =  Array();

    public function send( IPushableObserved $sender )
    {
        $payload = $sender->getPayload();

        $pushNotificationDecorator = new pushNotificationDecorator( self::TYPE, $payload );
        $data = $pushNotificationDecorator->getData( $payload );

        $this->sendNotification( self::QUEUE_NAME, $data );
    }

}

class PushableDeviceIpad extends AbstractPushableObject
{
    // possible late static binding, might have to do protected $type = 'iOS';
    const TYPE = 'iOS';
    const QUEUE_NAME = 'ipad_queue_name';
    public $er =  Array();

    public function send( IPushableObserved $sender )
    {
        $payload = $sender->getPayload();

        $pushNotificationDecorator = new pushNotificationDecorator( self::TYPE, $payload );
        $data = $pushNotificationDecorator->getData( $payload );

        $this->sendNotification( self::QUEUE_NAME, $data );
    }

}

class PushableDeviceAndroid extends AbstractPushableObject
{
    // possible late static binding, might have to do protected $type = 'iOS';
    const TYPE = 'android';
    const QUEUE_NAME = 'android_queue_name';
    public $er =  Array();

    public function send( IPushableObserved $sender )
    {
        $payload = $sender->getPayload();

        $pushNotificationDecorator = new pushNotificationDecorator( self::TYPE, $payload );
        $data = $pushNotificationDecorator->getData( $payload );

        $this->sendNotification( self::QUEUE_NAME, $data );
    }

}

class PushNotifications
{


    public function sendNotification( $payload )
    {

        // note: in mybPublish memberId is reserved, always present
        $memberId = $payload->memberId;

        // check rate limiting

        // check notifications settings


        // get list of devices for this member
        $userDeviceLinks = $this->getUserDeviceLinks( $memberId );

        // instantiate observable object
        $pushableObjectsList = new RemovableObjectsList();

        // add each device as an observer, each will handle the send in their own way
        foreach( $userDeviceLinks as $deviceKey => $deviceData )
        {
            list( $deviceType, $deviceId ) = explode( ':', $deviceKey );

            $className = 'PushableDevice' . ucfirst( $deviceType );
            // if class exists 
            $pushableDevice = new $className();
            $pushableObjectsList->addObserver( $pushableDevice );    
        }

        if ( ! $pushableObjectsList->push( $payload ) )
        {
            // handle error condition using $pushableObjectsList->err array 
        }
    }


}


?>

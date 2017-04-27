#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ),
  min_rtt(99999999),
  cwnd(20),
  decreased(false),
  delta_send(0),
  last_send(0),
  last_receive(0),
  packet_counter(0)
{
  debug_ = true;
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  //unsigned int the_window_size = 100;
  unsigned int the_window_size = cwnd;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
   << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
            /* of the sent datagram */
            const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  if(last_send == 0) {
    last_send = send_timestamp;
  } else {
    delta_send = send_timestamp - last_send;
    last_send = send_timestamp;
  }

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
   << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
             /* what sequence number was acknowledged */
             const uint64_t send_timestamp_acked,
             /* when the acknowledged datagram was sent (sender's clock) */
             const uint64_t recv_timestamp_acked,
             /* when the acknowledged datagram was received (receiver's clock)*/
             const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{

  if(last_receive == 0) {
    last_receive = timestamp_ack_received;
  } else {
    last_receive = timestamp_ack_received;
  }

  /* Default: take no action */
  float packet_rtt = timestamp_ack_received - send_timestamp_acked;

  if(packet_counter == 0) {
    min_rtt = packet_rtt;
  }

  packet_counter++;
  packet_counter %= 8000;

  if(packet_rtt < min_rtt) {
    min_rtt = packet_rtt;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
     << " received ack for datagram " << sequence_number_acked
     << " (send @ time " << send_timestamp_acked
     << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
     << endl;
  }

  uint64_t upper_thresh = min_rtt * 1.3;

  cerr << "Min RTT: " << min_rtt << endl;

  if(packet_rtt > upper_thresh) {
    if(cwnd > min_rtt * 1.1) {
      cwnd *= 0.99;
    } else if(cwnd > 5) {
      cwnd--;
    } else {
      if(packet_counter % 2 == 0) {
        cwnd++;
      }
    }
  } else {
    cwnd++;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 100; /* timeout of one second */
}
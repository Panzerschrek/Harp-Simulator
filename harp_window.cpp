#include <time.h>
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>

#include "harp_window.h"


const unsigned int c_window_width= 400;
const unsigned int c_window_height= 144;

const unsigned int c_harp_apertures_count= 10;
const unsigned int c_harp_aprture_modes_count= 2;

const unsigned char c_background_color[]= { 32, 32, 32 };
const unsigned char c_inactive_aperture_color[]= { 200, 190, 170 };
const unsigned char c_active_aperture_color[]= { 150, 148, 130 };
const unsigned char c_note_name_color[]= { 84, 80, 76 };

const unsigned int c_font_size= 14;
const char* c_font_name= "Arial";

const unsigned int c_slider_width= 32;

const char* note_name_table[c_harp_apertures_count][c_harp_aprture_modes_count]=
{
	{ "1C", "1D" },
	{ "1E", "1G" },
	{ "1G", "1B" },
	{ "2C", "2D" },
	{ "2E", "2F" },
	{ "2G", "2A" },
	{ "3C", "2B" },
	{ "3E", "3D" },
	{ "3G", "3F" },
	{ "4C", "3A" }
};

const Qt::Key note_key_table[c_harp_apertures_count][c_harp_aprture_modes_count]=
{
	{ Qt::Key_1, Qt::Key_Q },
	{ Qt::Key_2, Qt::Key_W },
	{ Qt::Key_3, Qt::Key_E },
	{ Qt::Key_4, Qt::Key_R },
	{ Qt::Key_5, Qt::Key_T },
	{ Qt::Key_6, Qt::Key_Y },
	{ Qt::Key_7, Qt::Key_U },
	{ Qt::Key_8, Qt::Key_I },
	{ Qt::Key_9, Qt::Key_O },
	{ Qt::Key_0, Qt::Key_P }
};

const char* note_key_name_table[c_harp_apertures_count][c_harp_aprture_modes_count]=
{
	{ "1", "Q" },
	{ "2", "W" },
	{ "3", "E" },
	{ "4", "R" },
	{ "5", "T" },
	{ "6", "Y" },
	{ "7", "U" },
	{ "8", "I" },
	{ "9", "O" },
	{"10", "P" },
};

static bool nota_state_table[c_harp_apertures_count][c_harp_aprture_modes_count]=
{
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false }
};

//static const Qt::Key


HarpWindow::HarpWindow()
	: slider_( Qt::Orientation::Vertical, this )
{
	resize( c_window_width, c_window_height );
	setMinimumSize( c_window_width, c_window_height );
	show();

	timer_.setInterval(15);
	timer_.setSingleShot(false);

	connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer_.start(10);

	slider_.setMinimum( 0 );
	slider_.setMaximum( 100 );
	slider_.setValue( 100 );
	connect(
		&slider_,
		&QSlider::valueChanged,
		this,
		[this](int value)
		{
			sound_out_.SetVolume( float(value) / 100.0f );
		} );
}

HarpWindow::~HarpWindow()
{
}

void HarpWindow::resizeEvent(QResizeEvent* event)
{
	event->accept();

	slider_.resize( c_slider_width, height() );
}

void HarpWindow::paintEvent(QPaintEvent* event)
{
	QPainter p(this);

	p.setPen( QColor( c_note_name_color[0], c_note_name_color[1], c_note_name_color[2] ) );
	p.setFont( QFont(c_font_name, c_font_size) );

	p.fillRect( 0, 0, size().width(), size().height(),
				QColor(c_background_color[0], c_background_color[1], c_background_color[2]) );

	const unsigned int c_edge_border_size= 8;
	const unsigned int c_inner_border_size= 4;

	unsigned int plates_start_x= slider_.width();

	unsigned int window_width=  size().width () - plates_start_x;
	unsigned int window_height= size().height();

	unsigned int plate_width=
		(window_width - c_edge_border_size*2 - (c_harp_apertures_count-1) * c_inner_border_size )
		/ c_harp_apertures_count;
	unsigned int plate_height=
		(window_height - c_edge_border_size*2 - (c_harp_aprture_modes_count-1) * c_inner_border_size )
		/ c_harp_aprture_modes_count;

	QColor active_color( c_active_aperture_color[0], c_active_aperture_color[1], c_active_aperture_color[2] );
	QColor inactive_color( c_inactive_aperture_color[0], c_inactive_aperture_color[1], c_inactive_aperture_color[2] );

	for( unsigned int x= 0; x< c_harp_apertures_count; x++ )
		for( unsigned int y= 0 ;y< c_harp_aprture_modes_count; y++ )
		{
			unsigned int r_x= plates_start_x + c_edge_border_size + (plate_width + c_inner_border_size) * x;
			unsigned int r_y= c_edge_border_size + (plate_height + c_inner_border_size) * y;
			p.fillRect(
				r_x,
				r_y,
				plate_width,
				plate_height,
				nota_state_table[x][y] ? active_color : inactive_color );

			p.drawText(
				QRectF(r_x, r_y, plate_width, plate_height),
				note_name_table[x][y], Qt::AlignHCenter | Qt::AlignTop );

			p.drawText(
				QRectF(r_x, r_y, plate_width, plate_height),
				note_key_name_table[x][y], Qt::AlignHCenter | Qt::AlignBottom );
		}
}

void HarpWindow::keyPressEvent(QKeyEvent* event)
{
	if ( event->isAutoRepeat() ) return;

	for( unsigned int x= 0; x< c_harp_apertures_count; x++ )
		for( unsigned int y= 0 ;y< c_harp_aprture_modes_count; y++ )
			if( event->nativeVirtualKey() == note_key_table[x][y] )
			{
				sound_out_.StartNote( x, y );
				nota_state_table[x][y]= true;
				event->accept();
				repaint();
			}
}

void HarpWindow::keyReleaseEvent(QKeyEvent* event)
{
	if ( event->isAutoRepeat() ) return;

	for( unsigned int x= 0; x< c_harp_apertures_count; x++ )
		for( unsigned int y= 0 ;y< c_harp_aprture_modes_count; y++ )
			if( event->nativeVirtualKey() == note_key_table[x][y] )
			{
				sound_out_.StopNote( x, y );
				nota_state_table[x][y]= false;
				event->accept();
				repaint();
			}
}

void HarpWindow::onTimer()
{
	static unsigned int prev_tick= 0;

	unsigned int new_tick= clock();

	sound_out_.Update( float(new_tick - prev_tick) / float(CLOCKS_PER_SEC) );

	prev_tick= new_tick;
}

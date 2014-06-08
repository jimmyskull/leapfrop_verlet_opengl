#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <valarray>

#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace constants {

const double e = 2.71828182845904523536;

const double G = 6.6738480e-11;

}  // namespace constants

namespace {

double kScale = 1;
		
void draw_sphere(double radius, int slices, int stacks)
{
	GLUquadric *quad_obj = gluNewQuadric();
	
	gluQuadricDrawStyle(quad_obj, GLU_FILL);
	gluQuadricNormals(quad_obj, GLU_SMOOTH);
	gluQuadricTexture(quad_obj, true);
	gluSphere(quad_obj, radius, stacks, slices);
}

class PolarView {
public:
	PolarView() = default;
	
	~PolarView() = default;

    void update() const
    {
        glTranslatef(-30, 0.0, -distance);
        glRotatef(-twist, 0.0, 0.0, 1.0);
        glRotatef(-incidence, 1.0, 0.0, 0.0);
        glRotatef(-azimuth, 0.0, 0.0, 1.0);
	}

    float distance = 0;
    float azimuth = 0;
    float incidence = 0;
    float twist = 0;
};
        
class Mouse {
public:
    void motion(int x, int y)
    {
        const float SPEED_FACTOR = 0.8;	
        int deltay = y - y_start;
        int deltax = x - x_start;
        if (left_down) {
            if (alt_down) {
                polar.distance += deltay * SPEED_FACTOR / 3.0;
            } else {
                polar.azimuth += deltax * SPEED_FACTOR;
                polar.incidence += deltay * SPEED_FACTOR;
			}
        } else {
			if (right_down)
				polar.twist += deltax * SPEED_FACTOR;
		}
        x_start = x;
        y_start = y;
        glutPostRedisplay();
	}
        
    void click(int button, int state, int x, int y)
    {
        if (button == GLUT_LEFT_BUTTON) {
            left_down = (state == GLUT_DOWN);
            if (left_down) {
                x_start = x;
                y_start = y;
			}
        } else {
			if (button == GLUT_RIGHT_BUTTON) {
				right_down = (state == GLUT_DOWN);
				if (right_down) {
					x_start = x;
					y_start = y;
				}
			}
		}

        if (glutGetModifiers() == GLUT_ACTIVE_ALT)
            alt_down = (state == GLUT_DOWN);
        
		if (button == 3 || button == 4)
			wheel(button);
	}
	
	const PolarView& polarview() const
	{
		return polar;
	}
	
private:
	void wheel(int button)
	{
		double scale;
		if (button == 3)
			scale = 1.1;
		else
			scale = 1 / 1.1;
		glScalef(scale, scale, scale);
		glutPostRedisplay();
	}	

    int x_start = 0;
    int y_start = 0;
    bool left_down = false;
    bool right_down = false;
    bool alt_down = false;
	PolarView polar;
};

class Lighting {
public:
	Lighting() = default;
	
	~Lighting() = default;
	
	void SetupLighting()
	{
		const float ambient[4] = {0.5f, 0.5f, 0.5f, 0.0};
		const float light0_pos[4] = {1.0, 1.0, 1.0, 0.0};

		glEnable(GL_LIGHTING);
	
		glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
		
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0f);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);
		
		glEnable(GL_LIGHT0);

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		
		//glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	}

};

class Textures {
public:
	Textures() = default;
	
	~Textures() = default;
	
	unsigned int texture(unsigned int index) const
	{
		return textures_[index];
	}
	
	unsigned int random_texture() const
	{
		if (textures_.empty())
			return -1;  // There weren't loaded any textures.
			
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<unsigned int> dist(0, textures_.size());
		
		return textures_[dist(mt)];
	}

	void LoadTextures()
	{
		const char *dir = "./textures/";
		const unsigned int dir_len = strlen(dir);
		DIR *p_dir;
		char *buffer;
		long buf_size = pathconf(dir, _PC_NAME_MAX);
		
		// If the system doesn't provide the PATH_MAX, let's assume a value.
		if (buf_size == -1) 
			buf_size = 255;
		
		buffer = new char[buf_size + 1];
		try { 
			if ((p_dir = opendir(dir)) != NULL) {
				struct dirent *entry;
				glEnable(GL_TEXTURE_2D);
				while ((entry = readdir(p_dir)) != NULL) {
					if (strcmp(entry->d_name, ".") == 0 
						|| strcmp(entry->d_name, "..") == 0
						|| entry->d_name[0] == '.')
						continue;
					*buffer = 0;
					strncat(buffer, dir, buf_size);
					strncat(buffer, entry->d_name, buf_size - dir_len);
					LoadTexture(std::string(buffer));
				}
			} else {
				throw errno;
			}
		} catch (int e) {
			fprintf(stderr, "Error reading %s: %s\n", dir, strerror(e));
		}
		delete[] buffer;
	}
		
private:
	void LoadTexture(const std::string& filename)
	{
		printf("Loading %s\n", filename.c_str());
		SDL_Surface *image = IMG_Load(filename.c_str());
		
		if (image == NULL) 
			throw 2;
		
		textures_.push_back(-1);
		GLuint *texture = textures_.data() + textures_.size() - 1;

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		assert(glIsTexture(*texture) == GL_TRUE);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, 
						GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
		SDL_FreeSurface(image);
	}
	
	std::vector<unsigned int> textures_;
};

struct double3 {
	double x, y, z;

	double3() : x(0), y(0), z(0) {}

	double3(double x, double y, double z) : x(x), y(y), z(z) {}

	double3(const double3& b) : x(b.x), y(b.y), z(b.z) {}

	~double3() {}

	inline
	double3& operator =(const double3& b) {
		x = b.x;
		y = b.y;
		z = b.z;
		return *this;
	}

	inline
	double3 operator +=(const double3& b) {
		*this  = *this + b;
		return *this;
	}

	inline
	const double3 operator -(const double3& b) const {
		return double3(x - b.x, y - b.y, z - b.z);
	}

	inline
	const double3 operator +(const double3& b) const {
		return double3(x + b.x, y + b.y, z + b.z);
	}

	inline
	const double3 operator /(double scalar) const {
		return double3(x / scalar, y / scalar, z / scalar);
	}

	inline
	const double3 operator *(double scalar) const {
		return double3(x * scalar, y * scalar, z * scalar);
	}

	inline
	double distance(void) const {
		return x * x + y * y + z * z + (constants::e * constants::e);
	}

	std::string str(void) const {
		std::stringstream ss;
		ss << std::setprecision(8) << x << " " << y << " " << z;
		return ss.str();
	}
};

class Star {
public:
	uint32_t hip = 0;
	double3 pos = double3(0, 0, 0);
	double3 vel = double3(0, 0, 0);
	double3 acceleration = double3(0, 0, 0);
	double3 prev_delta = double3(0, 0, 0);
	double mass = 0;
	double3 color;
	int texture_id;
	
	std::vector<double3> travelled_path;
	
	Star() : texture_id(-1)
	{
		PickPathColor();
	}

	void update_acceleration(const std::vector<Star>& __restrict stars) {
		double3 a(0, 0, 0);
		for (auto& star: stars) {
			double3 r(star.pos - pos);
			double dist = r.distance();

			a += r * (star.mass / (dist * sqrt(dist)));
		}
		acceleration = a * constants::G;
	}

	void move_star(double dt) {
		const double3 delta = vel * dt + acceleration * ((dt * dt) / 2.);
		travelled_path.push_back(double3(pos));
		pos += delta;
		vel = prev_delta / dt + acceleration * (dt / 2.);
		prev_delta = delta;
	}
	
	void draw() const
	{
		const float material[4] = {1, 1, 1, 0};
		
		glDisable(GL_LIGHTING);
		draw_path();
		glEnable(GL_LIGHTING);
		
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glEnable(GL_TEXTURE_2D);
		glTranslated(pos.x * kScale, pos.y * kScale, pos.z * kScale);
		draw_sphere(mass * .2 - std::log10(mass), 30, 30);
		glPopMatrix();
	}

	std::string str(void) const {
		std::stringstream ss;
		ss << pos.str() << mass;
		return ss.str();
	}
	
private:
	void PickPathColor()
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(0, 1);
		while (color.x + color.y + color.z < 0.5)
			color = double3(dist(mt), dist(mt), dist(mt));		
	}

	void draw_path() const
	{
		glPushMatrix();
		glColor3f(color.x, color.y, color.z);
		glBegin(GL_LINES);
		for (unsigned int i = 1; i < travelled_path.size(); i += 1) {
			const double3& a = travelled_path[i - 1] * kScale;
			const double3& b = travelled_path[i] * kScale;
			glVertex3f(a.x, a.y, a.z);
			glVertex3f(b.x, b.y, b.z);
		}
		glEnd();
		glPopMatrix();
	}
	
};

class StarList {
public:
	StarList() = default;
	
	~StarList() = default;

	void ReadInput()
	{
		size_t count;
		if (scanf("%zu", &count) != 1) {
			printf("Error: no input.\n");
			exit(EXIT_FAILURE);
		}
		stars_.resize(count);
		for (size_t i = 0; i < count; i++) {
			Star& s = stars_[i];
			int r = scanf("%u %le %le %le %le %le %le %le", &s.hip, &s.pos.x,
					&s.pos.y, &s.pos.z, &s.vel.x, &s.vel.y,	&s.vel.z, &s.mass);
			if (r != 8) {
				printf("Error: invalid input.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	void Step(unsigned int iterations, int elapsed_time_per_iteration)
	{
		for (unsigned int it = 0; it < iterations; it++) {
			unsigned int s = stars_.size();
			
			#pragma omp parallel for
			for (unsigned int i = 0; i < s; ++i) {
				Star& s = stars_[i];
				s.update_acceleration(stars_);
			}

			for (auto& s: stars_)
				s.move_star(static_cast<double>(elapsed_time_per_iteration));
		}
	}
		
	void Print() const
	{
		std::cout << "Star list:" << std::endl;
		for (auto& s: stars_)
			std::cout << s.str() << std::endl;
	}
	
	void distribute_textures(const Textures& textures)
	{
		for (auto& s: stars_)
			s.texture_id = textures.random_texture();
	}

	const std::vector<Star>& stars() const
	{
		return stars_;
	}
	
private:	
	std::vector<Star> stars_;
};

static void DrawCallback();
static void MouseCallback(int button, int state, int x, int y);
static void MouseMotionCallback(int x, int y);
static void IdleCallback();
static void KeyboardCallback(unsigned char key, int x, int y);

class Simulation {
public:
	static Simulation& instance()
	{
		static Simulation instance;

		return instance;
	}

	~Simulation() = default;
	
	void set_star_list(StarList* list)
	{
		list_ = list;
		list_->distribute_textures(textures_);
	}
	
	Mouse& mouse()
	{
		return mouse_;
	}
	
	void work()
	{
		assert(list_ != NULL);
		fprintf(stderr, "loop\n");
		glutMainLoop();
	}
	
	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glPushMatrix();
		glTranslated(-30, 0, 0);
		mouse().polarview().update();
		
		for (auto& star: list_->stars()) {
			star.draw();
		}
		
		glPopMatrix();
    
		glutSwapBuffers();
	}
	
	void idle()
	{
		list_->Step(1, 2000);
		glutPostRedisplay();
	}
	
	void keyboard(unsigned char key, int /*x*/, int /*y*/) const
	{
		if (key == 'a')
			glTranslatef(5, 0, 0);
		if (key == 'd')
			glTranslatef(-5, 0, 0);
		if (key == 'w')
			glTranslatef(0, -5, 0);
		if (key == 's')
			glTranslatef(0, 5, 0);
		if (key == 'f')
			glTranslatef(0, 0, -10);
		if (key == 'r')
			glTranslatef(0, 0, 10);
		if (key == 'q')
			exit(0);
		glutPostRedisplay();
	}

private:
	Simulation(const Simulation&) = delete;
	
	Simulation() : list_(NULL)
	{
		SetupWindow("Simulation");
		InitGFX();
		InstallCallbacks();
		textures_.LoadTextures();
		lighting_.SetupLighting();
		fprintf(stderr, "GFX Initialized");
	}	

	void SetupWindow(const std::string& name)
	{
		glutInitWindowPosition(200, 50);
		glutInitWindowSize(1600, 900);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		glutCreateWindow(name.c_str());
	}

	void InitGFX()
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glMatrixMode(GL_PROJECTION);
		gluPerspective(45.0, 16.0 / 9.0, 0.1, 1000.0);
		gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		glMatrixMode(GL_MODELVIEW);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glLineWidth(.8);
		glEnable(GL_LINE_SMOOTH);
	}
	
	void InstallCallbacks()
	{
		glutDisplayFunc(DrawCallback);
		glutMouseFunc(MouseCallback);
		glutMotionFunc(MouseMotionCallback);
		glutIdleFunc(IdleCallback);
		glutKeyboardFunc(KeyboardCallback);
	}
	
	StarList* list_;
	Mouse mouse_;
	Textures textures_;
	Lighting lighting_;
};

void DrawCallback()
{
	Simulation::instance().draw();
}

void MouseCallback(int button, int state, int x, int y)
{
	Simulation::instance().mouse().click(button, state, x, y);
}

void MouseMotionCallback(int x, int y)
{
	Simulation::instance().mouse().motion(x, y);
}

void IdleCallback()
{
	Simulation::instance().idle();
}

void KeyboardCallback(unsigned char key, int x, int y)
{
	Simulation::instance().keyboard(key, x, y);
}
	
}  // namespace

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	Simulation& simulator = Simulation::instance();
	StarList list;

		fprintf(stderr, "loop\n");
		list.ReadInput();
		fprintf(stderr, "loop\n");
	simulator.set_star_list(&list);
	simulator.work();
	
	return EXIT_SUCCESS;
}

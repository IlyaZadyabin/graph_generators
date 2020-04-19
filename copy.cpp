#include <iostream>
#include <cmath>
#include <new>
#include <iterator>
#include <algorithm>
#include <map>
#include <random>
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
#include <omp.h>
#include <sstream>

enum { BORDER = 250, THREADS = 8};

class copy_graph {
private:
    std::map <int, std::vector<int>> neighbours;
public:
    int *src_ids, *dst_ids, additional_vertexes, current_vertexes;
    long long degree; double prob; long long edge = 0;
    double generation_time = 0;

    explicit copy_graph(int *src, int *dst, int add_vert, int orig_vert, int d, double p):
        src_ids(src), dst_ids(dst), additional_vertexes(add_vert), current_vertexes(orig_vert),
            degree(d), prob(p){
        for(int i = 0; i < current_vertexes; i++) {
            neighbours.insert(std::map<int, std::vector<int>>::value_type(i, std::vector <int>{}));
        }
    }

    void generation(){
        double start_time = omp_get_wtime();

        std::random_device rd1;
        std::mt19937 generator(rd1());
        std::bernoulli_distribution distribution(prob);

        std::random_device rd2;
        std::mt19937 eng(rd2());

        for(int i = current_vertexes; i < current_vertexes + additional_vertexes; i++) {
            neighbours.insert(std::map<int, std::vector<int>>::value_type(i, std::vector <int>{}));
            std::uniform_int_distribution <int> distr(0, i - 1);
            for(int j = 0; j < degree; j++){
                src_ids[edge] = i;
                dst_ids[edge] = distribution(generator) ? distr(eng) : neighbours[distr(eng)][j];
                neighbours[i].push_back(dst_ids[edge]);
                edge++;
            }
        }
        current_vertexes += additional_vertexes;
        double end_time = omp_get_wtime();
        generation_time = end_time - start_time;
    }

    void fill_map(){
        std::string buf, arr;
        char c;
        int num = 0, vertex = 0;
        for(int i = 0; i < 3; i++){
            std::getline(std::cin, buf);
        }

        for(int i = 0; i < current_vertexes; i++){
            std::getline(std::cin, buf);
            std::stringstream ss;
            ss << buf;
            ss >> vertex >> c;
            while(ss >> num){
                neighbours[i].push_back(num - 1);
            }
        }
    }

    void generate_regular_graph(){
        fill_map();
        omp_set_num_threads(THREADS);
#pragma omp parallel for
        for(int i = 0; i < current_vertexes; i++){
            for(int j = 0; j < (int) neighbours[i].size(); j++){
                if(neighbours[i][j] > i){
                    src_ids[edge] = i;
                    dst_ids[edge] = neighbours[i][j];
                    edge++;
                }
            }
        }
    }

    void print_edge_list(FILE *file){
        fprintf(file, "digraph G {\nratio=0.8\n");
        for (long long e = 0; e < edge; e++) {
            fprintf(file, "%d->%d\n", src_ids[e], dst_ids[e]);
        }
        fprintf(file, "}");
    }

    bool create_png(){
        double start_time = omp_get_wtime();
        FILE *f = fopen("dot", "w");
        print_edge_list(f);
        fclose(f);

        GVC_t *gvc = gvContext();
        FILE *fp = fopen("dot", "r");
        Agraph_t *g = agread(fp, nullptr);
        gvLayout(gvc, g, "dot");
        gvRender(gvc, g, "png", fopen("picture.png", "w"));
        gvFreeLayout(gvc, g);
        agclose(g);
        double end_time = omp_get_wtime();
        printf("Rendering time = %f\n", end_time - start_time);
        return (gvFreeContext(gvc));
    }
};

void graph_generator_copy(int *src_ids, int *dst_ids, int additional_vertexes,
        int orig_vertexes, int degree, double prob){

    copy_graph G(src_ids, dst_ids, additional_vertexes, orig_vertexes, degree, prob);
    G.generate_regular_graph();
    G.generation();
    printf("Generation time = %f\n", G.generation_time);
    printf("Amount of edges = %lld\n", G.edge);
    if(G.current_vertexes <= BORDER) {
        G.create_png();
    }
}

int main(int argc, char **argv) //additional_vertexes, reg_graph_vertexes, degree, probability_of_coin_toss
{
    int add_vert = strtol(argv[1], nullptr, 10);
    int reg_graph_vert = strtol(argv[2], nullptr, 10);
    int degree = strtol(argv[3], nullptr, 10);
    double probability = strtod(argv[4], nullptr);

    int V = reg_graph_vert + add_vert;
    long long max_edge = V * (V - 1) / 2;

    int *src_ids = new int[max_edge];
    int *dst_ids = new int[max_edge];

    graph_generator_copy(src_ids, dst_ids, add_vert, reg_graph_vert, degree, probability);

    delete []src_ids;
    delete []dst_ids;
}
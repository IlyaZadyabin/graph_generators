#include <iostream>
#include <cmath>
#include <new>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unistd.h>
#include <map>
#include <random>
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
#include <omp.h>

enum { THREADS = 128, BORDER = 250 };

class graph {
private:
    int vertex;
    double prob;
    int *src_ids, *dst_ids;
public:
    long long edge = 0;
    double generation_time = 0;

    explicit graph(int v = 0, double p = 1, int *s = nullptr, int *d = nullptr):
            vertex(v), prob(p), src_ids(s), dst_ids(d) {

        std::default_random_engine generator;
        std::bernoulli_distribution distribution(prob);

        //Adding values to edge's arrays
        double start_time = omp_get_wtime();
        int v1 = 0;
        omp_set_num_threads(THREADS);
        int *tmp_src = src_ids;
        int *tmp_dst = dst_ids;
#pragma omp parallel shared(edge) private(v1)
        {
            std::vector <int> thread_src, thread_dst;
#pragma omp for schedule(guided)
            for (v1 = 0; v1 < vertex; v1++) {
                for (int v2 = v1 + 1; v2 < vertex; v2++) {
                    if (distribution(generator)) {
                        thread_src.push_back(v1);
                        thread_dst.push_back(v2);
                    }
                }
            }
#pragma omp critical
            {
                std::copy(thread_src.begin(), thread_src.end(), tmp_src);
                tmp_src += thread_src.size();
                edge += thread_src.size();
                thread_src.resize(0);

                std::copy(thread_dst.begin(), thread_dst.end(), tmp_dst);
                tmp_dst += thread_dst.size();
                thread_dst.resize(0);
            }
        }
        double end_time = omp_get_wtime();
        generation_time = end_time - start_time;
    }

    ~graph() = default;

    static void print_edge_list(FILE *file, int *src_ids, int *dst_ids, long long edge){
        fprintf(file, "digraph G {\nratio=0.8\n");
        for (long long e = 0; e < edge; e++) {
            fprintf(file, "%d->%d\n", src_ids[e], dst_ids[e]);
        }
        fprintf(file, "}");
    }

    static bool create_png(int *src_ids, int *dst_ids, long long edge){
        double start_time = omp_get_wtime();
        FILE *f = fopen("dot", "w");
        graph::print_edge_list(f, src_ids, dst_ids, edge);
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

void graph_generator_reny(int *src_ids, int *dst_ids, int V, long long *E, double prob){
    graph A(V, prob, src_ids, dst_ids);
    *E = A.edge;
    printf("Generation time = %f\n", A.generation_time);
    printf("Amount of edges = %lld\n", A.edge);
}

int main(int argc, char **argv) // amount_of_vertexes, probability
{
    int V = strtol(argv[1], nullptr, 10);
    double probability = strtod(argv[2], nullptr);
    long long edge_cnt = 0;
    long long max_edge = V * (V - 1) / 2;
    int *src_ids = new int[max_edge];
    int *dst_ids = new int[max_edge];

    graph_generator_reny(src_ids, dst_ids, V, &edge_cnt, probability);
    //graph_generator_reny(src_ids, dst_ids, V, &edge_cnt, 0.5 * log(V) / V);
    //graph_generator_reny(src_ids, dst_ids, V, &edge_cnt, 1.2 * log(V) / V);
    //graph_generator_reny(src_ids, dst_ids, V, &edge_cnt, 0.3 / V);
    //graph_generator_reny(src_ids, dst_ids, V, &edge_cnt, 1.1 / V);

    if(V <= BORDER){
        graph::create_png(src_ids, dst_ids, edge_cnt);
    }

    delete []src_ids;
    delete []dst_ids;
}
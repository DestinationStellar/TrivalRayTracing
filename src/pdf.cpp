#include "pdf.hpp"
#include "group.hpp"

double HittablePDF::value(const Vector3f& direction) const {
    return list->pdf_value(o, direction);
}

Vector3f HittablePDF::generate() const {
    return list->random(o);
}
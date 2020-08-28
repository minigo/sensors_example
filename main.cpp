#include <QCoreApplication>
#include <QtCore/QDebug>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <sensors/sensors.h>
#include <sensors/error.h>

QString sensors ()
{
    const sensors_chip_name *chip {nullptr};
    int chip_nr {0};

    if (sensors_init (nullptr) != 0) {
        qCritical () << "Error while 'sensors_init'";
        sensors_cleanup ();
        return "";
    }

    QJsonObject json;

    while ((chip = sensors_get_detected_chips (nullptr, &chip_nr)))
    {
        QJsonObject jsonChip;
        static char buf [300];

        if (sensors_snprintf_chip_name (buf, 300, chip) < 0) {
            qCritical () << "Error while 'sensors_snprintf_chip_name'";
            continue;
        }

        const char *adap = sensors_get_adapter_name (&chip->bus);
        if (!adap) {
            qCritical () << "Error while 'sensors_get_adapter_name'";
            continue;
        }
        jsonChip.insert ("Adapter", adap);

        //------------------------------------------------------
        int a, b, err;
        const sensors_feature *feature {nullptr};
        const sensors_subfeature *sub {nullptr};

        double val;

        a = 0;
        while ((feature = sensors_get_features (chip, &a)))
        {
            char *label {nullptr};

            if (!(label = sensors_get_label (chip, feature)))
            {
                qWarning () << "Can't get label of feature" << feature->name;
                continue;
            }

            b = 0;
            QJsonObject jsonSub;

            while ((sub = sensors_get_all_subfeatures (chip, feature, &b)))
            {
                if (sub->flags & SENSORS_MODE_R)
                {
                    if ((err = sensors_get_value (chip, sub->number,& val)))
                    {
                        qWarning () << "Can't get value of subfeature"
                                    << sub->name << sensors_strerror (err);
                    } else {
                        jsonSub.insert (sub->name, val);
                    }
                }
            }

            jsonChip.insert (label, jsonSub);
            free (label);
        }
        //------------------------------------------------------

        json.insert (buf, jsonChip);
    }

    sensors_cleanup ();

    QJsonDocument doc (json);
    return doc.toJson (QJsonDocument::Compact);
}

int main (int argc, char *argv[])
{
    QCoreApplication a (argc, argv);
    qDebug () << sensors ();
    return 0;
}

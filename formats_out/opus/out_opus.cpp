/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "out_opus.h"
#include <QDebug>

static const constexpr char *BITRATE_TYPE_KEY = "BitrateType";
static const constexpr char *BITRATE_KEY      = "Bitrate";

/************************************************

 ************************************************/
OutFormat_Opus::OutFormat_Opus()
{
    mId      = "OPUS";
    mExt     = "opus";
    mName    = "Opus";
    mOptions = FormatOption::NoOptions;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Opus::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert(BITRATE_TYPE_KEY, "VBR");
    res.insert(BITRATE_KEY, 96);
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Opus::configPage(QWidget *parent) const
{
    return new ConfigPage_Opus(parent);
}

/************************************************

 ************************************************/
Conv::Encoder *OutFormat_Opus::createEncoder() const
{
    return new Encoder_Opus();
}

/************************************************

 ************************************************/
ConfigPage_Opus::ConfigPage_Opus(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    opusBitrateTypeCbx->addItem(tr("VBR - variable bitrate", "Opus encoding mode"), "VBR");
    opusBitrateTypeCbx->addItem(tr("CVBR - constrained variable bitrate", "Opus encoding mode"), "CVBR");

    opusBitrateTypeCbx->setToolTip(toolTipCss() + opusBitrateTypeCbx->toolTip());
    opusBitrateTypeLabel->setToolTip(opusBitrateTypeCbx->toolTip());

    opusBitrateSlider->setToolTip(toolTipCss() + opusBitrateSlider->toolTip());
    opusBitrateSpin->setToolTip(opusBitrateSlider->toolTip());
    opusBitrateLabel->setToolTip(opusBitrateSlider->toolTip());
}

/************************************************

 ************************************************/
void ConfigPage_Opus::load(const Profile &profile)
{
    loadWidget(profile, BITRATE_TYPE_KEY, opusBitrateTypeCbx);
    loadWidget(profile, BITRATE_KEY, opusBitrateSlider);
}

/************************************************

 ************************************************/
void ConfigPage_Opus::save(Profile *profile)
{
    saveWidget(profile, BITRATE_TYPE_KEY, opusBitrateTypeCbx);
    saveWidget(profile, BITRATE_KEY, opusBitrateSlider);
}

/************************************************

 ************************************************/
QStringList Encoder_Opus::programArgs() const
{
    QStringList args;

    args << programPath();

    args << "--quiet";

    QString type = profile().value(BITRATE_TYPE_KEY).toString();
    if (type == "VBR")
        args << "--vbr";

    if (type == "CVBR")
        args << "--cvbr";

    args << "--bitrate" << profile().value(BITRATE_KEY).toString();

    // Tags .....................................................
    if (!track().artist().isEmpty())
        args << "--artist" << track().artist();
    if (!track().album().isEmpty())
        args << "--album" << track().album();
    if (!track().genre().isEmpty())
        args << "--genre" << track().genre();
    if (!track().date().isEmpty())
        args << "--date" << track().date();
    if (!track().title().isEmpty())
        args << "--title" << track().title();
    if (!track().comment().isEmpty())
        args << "--comment" << QString("COMMENT=%1").arg(track().comment());
    if (!track().discId().isEmpty())
        args << "--comment" << QString("DISCID=%1").arg(track().discId());
    if (!track().tag(TagId::AlbumArtist).isEmpty()) {
        args << "--comment" << QString("album_artist=%1").arg(track().tag(TagId::AlbumArtist));
    }

    args << "--comment" << QString("tracknumber=%1").arg(track().trackNum());
    args << "--comment" << QString("tracktotal=%1").arg(track().trackCount());

    args << "--comment" << QString("disc=%1").arg(track().discNum());
    args << "--comment" << QString("discnumber=%1").arg(track().discNum());
    args << "--comment" << QString("disctotal=%1").arg(track().discCount());

    if (!coverFile().isEmpty()) {
        args << "--picture" << coverFile();
    }

    // Files ....................................................
    args << "-";
    args << outFile();

    return args;
}

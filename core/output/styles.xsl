<?xml version="1.0"?>

<!-- TestFarm Report default CSS

This file is part of TestFarm,
the Test Automation Tool for Embedded Software.
Please visit http://www.testfarm.org.

TestFarm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TestFarm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="styles-custom-file" select="'/var/testfarm/lib/styles.xml'"/>
<xsl:param name="styles-standard-file" select="'/opt/testfarm/lib/styles.xml'"/>

<xsl:variable name="styles-file">
  <xsl:choose>
    <xsl:when test="document($styles-custom-file)">
      <xsl:value-of select="$styles-custom-file"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$styles-standard-file"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:template name="styles">
  <xsl:copy-of select="document($styles-file)"/>
</xsl:template>

</xsl:stylesheet>

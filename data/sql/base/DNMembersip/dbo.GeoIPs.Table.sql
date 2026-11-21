USE [dnmembership]
GO
/****** Object:  Table [dbo].[GeoIPs]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GeoIPs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[GeoIPs](
	[StartIP] [int] NOT NULL,
	[EndIP] [int] NOT NULL,
	[CountryName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
 CONSTRAINT [PK_GeoIPs] PRIMARY KEY CLUSTERED 
(
	[StartIP] ASC,
	[EndIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO

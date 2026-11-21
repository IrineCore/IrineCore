USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_WeeklyUV]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_WeeklyUV](
	[BasisDate] [date] NOT NULL,
	[UVCount] [int] NULL,
	[CharacterLevel] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_WeeklyUV_BasisDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_WeeklyUV_BasisDate] ON [dbo].[ST_WeeklyUV]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

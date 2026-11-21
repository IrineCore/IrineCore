USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyLevelSurvival]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyLevelSurvival](
	[BasisDate] [date] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CreateMonth] [date] NOT NULL,
	[RU] [int] NOT NULL,
	[UVDay] [int] NOT NULL,
	[UVMonth] [int] NOT NULL,
	[UVWeek] [int] NOT NULL,
	[RevisitCount] [int] NULL,
	[NationalityCode] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailyLevelSurvival_BasisDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailyLevelSurvival_BasisDate] ON [dbo].[ST_DailyLevelSurvival]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

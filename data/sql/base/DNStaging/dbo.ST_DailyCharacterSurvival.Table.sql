USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyCharacterSurvival]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyCharacterSurvival](
	[BasisDate] [date] NOT NULL,
	[WorldID] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CreateMonth] [date] NOT NULL,
	[CharacterCreateCount] [int] NOT NULL,
	[UniqueCharacterDay] [int] NOT NULL,
	[UniqueCharacterWeek] [int] NOT NULL,
	[UniqueCharacterMonth] [int] NOT NULL,
	[RevisitCount] [int] NULL,
	[NationalityCode] [tinyint] NULL,
	[FirstCreateCharacterFlag] [bit] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailyCharacterSurvival_BasisDate_WorldID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailyCharacterSurvival_BasisDate_WorldID] ON [dbo].[ST_DailyCharacterSurvival]
(
	[BasisDate] ASC,
	[WorldID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
GO

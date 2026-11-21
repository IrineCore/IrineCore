USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_JobAndLevelType6]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_JobAndLevelType6](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterCount] [int] NOT NULL
) ON [PRIMARY]
GO

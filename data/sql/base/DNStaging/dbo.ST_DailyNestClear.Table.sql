USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyNestClear]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyNestClear](
	[BasisDate] [date] NOT NULL,
	[MapID] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[ChallengeCount] [int] NOT NULL,
	[ClearCount] [int] NOT NULL,
 CONSTRAINT [PK_ST_DailyNestClear] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[MapID] ASC,
	[JobCode] ASC,
	[CharacterLevel] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO

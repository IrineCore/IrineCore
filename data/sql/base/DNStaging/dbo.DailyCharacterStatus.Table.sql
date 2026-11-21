USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailyCharacterStatus]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyCharacterStatus](
	[BasisDate] [date] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterExp] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_DailyCharacterStatus] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO

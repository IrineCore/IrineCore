USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarOpeningPointDailyTotal]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarOpeningPointDailyTotal](
	[BasisDate] [date] NOT NULL,
	[Ranking] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[OpeningPointSum] [int] NOT NULL,
	[GuildID] [int] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
 CONSTRAINT [PK_GuildWarOpeningPointDailyTotal] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[Ranking] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO

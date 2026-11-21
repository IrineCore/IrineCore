USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarOpeningPointGuildTotal]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarOpeningPointGuildTotal](
	[GuildWarMissionTypeCode] [tinyint] NOT NULL,
	[Ranking] [int] NOT NULL,
	[GuildID] [int] NOT NULL,
	[OpeningPointSum] [int] NOT NULL,
	[TeamColorCode] [tinyint] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
 CONSTRAINT [PK_GuildWarOpeningPointGuildTotal] PRIMARY KEY CLUSTERED 
(
	[GuildWarMissionTypeCode] ASC,
	[Ranking] ASC,
	[GuildID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO

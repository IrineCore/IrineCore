USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarRewardForCharacter]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarRewardForCharacter](
	[CharacterID] [bigint] NOT NULL,
	[GuildWarScheduleID] [smallint] NOT NULL,
	[RewardItem] [bit] NOT NULL,
	[RewardMail] [bit] NOT NULL,
	[RewardPointByOpening] [bit] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [PK_GuildWarRewardForCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[GuildWarRewardForCharacter] ADD  CONSTRAINT [PK_GuildWarRewardForCharacter] PRIMARY KEY NONCLUSTERED 
(
	[CharacterID] ASC,
	[GuildWarScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO

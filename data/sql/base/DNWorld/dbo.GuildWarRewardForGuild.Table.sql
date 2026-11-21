USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarRewardForGuild]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarRewardForGuild](
	[GuildID] [int] NOT NULL,
	[GuildWarScheduleID] [smallint] NOT NULL,
	[RewardSkill] [bit] NOT NULL,
	[RewardItem] [bit] NOT NULL,
	[RewardMail] [bit] NOT NULL,
	[RewardPointByOpening] [bit] NOT NULL,
	[RewardPointByFinal] [bit] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [PK_GuildWarRewardForGuild]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[GuildWarRewardForGuild] ADD  CONSTRAINT [PK_GuildWarRewardForGuild] PRIMARY KEY NONCLUSTERED 
(
	[GuildID] ASC,
	[GuildWarScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[GuildWarRewardForGuild]  WITH CHECK ADD  CONSTRAINT [FK_Guilds_GuildWarRewardForGuild_GuildID] FOREIGN KEY([GuildID])
REFERENCES [dbo].[Guilds] ([GuildID])
GO
ALTER TABLE [dbo].[GuildWarRewardForGuild] CHECK CONSTRAINT [FK_Guilds_GuildWarRewardForGuild_GuildID]
GO

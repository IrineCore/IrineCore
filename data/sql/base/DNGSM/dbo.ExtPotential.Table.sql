USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtPotential]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtPotential](
	[id] [int] NULL,
	[_PotentialID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PotentialNo] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PotentialRatio] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PrefixNameID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State1Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State2Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State3Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State4Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State5Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State6Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State7Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State8] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State8Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State9] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State9Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State10Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State11] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State11Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State12] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State12Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State13] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State13Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State14] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State14Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State15] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State15Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State16] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State16Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillUsingType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtPotential_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtPotential_id] ON [dbo].[ExtPotential]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

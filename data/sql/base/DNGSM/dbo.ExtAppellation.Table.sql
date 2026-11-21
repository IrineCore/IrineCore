USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtAppellation]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtAppellation](
	[id] [int] NULL,
	[_NameID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LevelLimit] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DescriptionID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State8] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue8] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State9] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue9] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateValue10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Color] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Type] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ColorValue] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillLinkItemID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UIString_NameID] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtAppellation_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtAppellation_id] ON [dbo].[ExtAppellation]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

USE [dnmembership]
GO
/****** Object:  Table [dbo].[TemporaryBlock]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TemporaryBlock]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[TemporaryBlock](
	[TemporaryBlockID] [int] IDENTITY(1,1) NOT NULL,
	[StartDate] [smalldatetime] NOT NULL,
	[EndDate] [smalldatetime] NOT NULL,
	[AdminID] [int] NOT NULL,
	[RegisterDate] [smalldatetime] NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
	[DeleteDate] [smalldatetime] NULL,
 CONSTRAINT [PK_TemporaryBlock] PRIMARY KEY CLUSTERED 
(
	[TemporaryBlockID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_TemporaryBlock_StartDate_EndDate_DeleteFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[TemporaryBlock]') AND name = N'IX_NN_TemporaryBlock_StartDate_EndDate_DeleteFlag')
CREATE NONCLUSTERED INDEX [IX_NN_TemporaryBlock_StartDate_EndDate_DeleteFlag] ON [dbo].[TemporaryBlock]
(
	[StartDate] ASC,
	[EndDate] ASC,
	[DeleteFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNF_TemporaryBlock_DeleteFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[TemporaryBlock]') AND name = N'IX_NNF_TemporaryBlock_DeleteFlag')
CREATE NONCLUSTERED INDEX [IX_NNF_TemporaryBlock_DeleteFlag] ON [dbo].[TemporaryBlock]
(
	[DeleteFlag] ASC
)
WHERE ([DeleteFlag]=CONVERT([bit],(0),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO

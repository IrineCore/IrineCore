USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventConditionForPetal]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventConditionForPetal]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventConditionForPetal](
	[EventConditionID] [int] IDENTITY(1,1) NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[FirstPetalAmount] [int] NULL,
	[PetalIncomeCountLimit] [int] NULL,
	[ConsecutionBonusCount] [int] NULL,
	[ConsecutionBonusPetalAmount] [int] NULL,
 CONSTRAINT [PK_EventConditionForPetal] PRIMARY KEY CLUSTERED 
(
	[EventConditionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_EventCondition_RegisterDate]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[EventConditionForPetal] ADD  CONSTRAINT [DF_EventCondition_RegisterDate]  DEFAULT (getdate()) FOR [RegisterDate]
END
GO
